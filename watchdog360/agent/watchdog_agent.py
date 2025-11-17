#!/usr/bin/env python3
"""
Watchdog360 Monitoring Agent
Collects system metrics and sends to Watchdog360 backend
"""

import os
import sys
import time
import json
import psutil
import requests
import logging
import socket
from datetime import datetime
from typing import Dict, List, Any
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

# Configuration
CONFIG_FILE = "/etc/watchdog360/config.json"
LOG_FILE = "/var/log/watchdog360/agent.log"
PID_FILE = "/var/run/watchdog360.pid"

# Setup logging
os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(LOG_FILE),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger("watchdog360-agent")


class MetricsCollector:
    """Collects system performance metrics"""

    def __init__(self):
        self.hostname = socket.gethostname()
        self.boot_time = datetime.fromtimestamp(psutil.boot_time())

    def get_cpu_metrics(self) -> Dict[str, Any]:
        """Get CPU usage metrics"""
        cpu_percent = psutil.cpu_percent(interval=1, percpu=False)
        cpu_count = psutil.cpu_count()
        cpu_freq = psutil.cpu_freq()

        return {
            "cpu_percent": cpu_percent,
            "cpu_count": cpu_count,
            "cpu_freq_current": cpu_freq.current if cpu_freq else 0,
            "cpu_freq_min": cpu_freq.min if cpu_freq else 0,
            "cpu_freq_max": cpu_freq.max if cpu_freq else 0,
            "load_average": list(psutil.getloadavg()) if hasattr(psutil, 'getloadavg') else [0, 0, 0]
        }

    def get_memory_metrics(self) -> Dict[str, Any]:
        """Get memory usage metrics"""
        virtual_mem = psutil.virtual_memory()
        swap_mem = psutil.swap_memory()

        return {
            "memory_total": virtual_mem.total,
            "memory_available": virtual_mem.available,
            "memory_used": virtual_mem.used,
            "memory_percent": virtual_mem.percent,
            "swap_total": swap_mem.total,
            "swap_used": swap_mem.used,
            "swap_percent": swap_mem.percent
        }

    def get_disk_metrics(self) -> List[Dict[str, Any]]:
        """Get disk usage metrics"""
        disk_metrics = []
        partitions = psutil.disk_partitions()

        for partition in partitions:
            try:
                usage = psutil.disk_usage(partition.mountpoint)
                disk_metrics.append({
                    "device": partition.device,
                    "mountpoint": partition.mountpoint,
                    "fstype": partition.fstype,
                    "total": usage.total,
                    "used": usage.used,
                    "free": usage.free,
                    "percent": usage.percent
                })
            except PermissionError:
                continue

        return disk_metrics

    def get_io_metrics(self) -> Dict[str, Any]:
        """Get I/O metrics"""
        io_counters = psutil.disk_io_counters()

        if io_counters:
            return {
                "read_count": io_counters.read_count,
                "write_count": io_counters.write_count,
                "read_bytes": io_counters.read_bytes,
                "write_bytes": io_counters.write_bytes,
                "read_time": io_counters.read_time,
                "write_time": io_counters.write_time
            }
        return {}

    def get_network_metrics(self) -> Dict[str, Any]:
        """Get network traffic metrics"""
        net_io = psutil.net_io_counters()

        return {
            "bytes_sent": net_io.bytes_sent,
            "bytes_recv": net_io.bytes_recv,
            "packets_sent": net_io.packets_sent,
            "packets_recv": net_io.packets_recv,
            "errin": net_io.errin,
            "errout": net_io.errout,
            "dropin": net_io.dropin,
            "dropout": net_io.dropout
        }

    def get_top_processes(self, limit: int = 10) -> List[Dict[str, Any]]:
        """Get top processes by CPU and memory usage"""
        processes = []

        for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_percent', 'username']):
            try:
                pinfo = proc.info
                processes.append({
                    "pid": pinfo['pid'],
                    "name": pinfo['name'],
                    "cpu_percent": pinfo['cpu_percent'] or 0,
                    "memory_percent": pinfo['memory_percent'] or 0,
                    "username": pinfo['username']
                })
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass

        # Sort by CPU usage
        processes.sort(key=lambda x: x['cpu_percent'], reverse=True)
        return processes[:limit]

    def collect_all_metrics(self) -> Dict[str, Any]:
        """Collect all system metrics"""
        return {
            "timestamp": datetime.utcnow().isoformat(),
            "hostname": self.hostname,
            "cpu": self.get_cpu_metrics(),
            "memory": self.get_memory_metrics(),
            "disk": self.get_disk_metrics(),
            "io": self.get_io_metrics(),
            "network": self.get_network_metrics(),
            "processes": self.get_top_processes(10)
        }


class MetricsPusher:
    """Pushes metrics to Watchdog360 backend with retry mechanism"""

    def __init__(self, api_url: str, token: str):
        self.api_url = api_url.rstrip('/')
        self.token = token
        self.session = self._create_session()

    def _create_session(self) -> requests.Session:
        """Create requests session with retry mechanism"""
        session = requests.Session()

        # Retry strategy: 3 retries with exponential backoff
        retry_strategy = Retry(
            total=3,
            backoff_factor=2,  # 2s, 4s, 8s
            status_forcelist=[429, 500, 502, 503, 504],
            allowed_methods=["POST"]
        )

        adapter = HTTPAdapter(max_retries=retry_strategy)
        session.mount("http://", adapter)
        session.mount("https://", adapter)

        return session

    def push_metrics(self, metrics: Dict[str, Any]) -> bool:
        """Push metrics to backend API"""
        headers = {
            "Authorization": f"Bearer {self.token}",
            "Content-Type": "application/json"
        }

        try:
            response = self.session.post(
                f"{self.api_url}/api/v1/metrics",
                json=metrics,
                headers=headers,
                timeout=10
            )

            if response.status_code == 200:
                logger.debug("Metrics pushed successfully")
                return True
            else:
                logger.error(f"Failed to push metrics: {response.status_code} - {response.text}")
                return False

        except requests.exceptions.RequestException as e:
            logger.error(f"Error pushing metrics: {e}")
            return False


class WatchdogAgent:
    """Main agent class"""

    def __init__(self, config_path: str = CONFIG_FILE):
        self.config = self._load_config(config_path)
        self.collector = MetricsCollector()
        self.pusher = MetricsPusher(
            self.config['api_url'],
            self.config['token']
        )
        self.interval = self.config.get('interval', 10)  # Default 10 seconds

    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            logger.error(f"Configuration file not found: {config_path}")
            sys.exit(1)
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in config file: {e}")
            sys.exit(1)

    def _write_pid_file(self):
        """Write PID file"""
        os.makedirs(os.path.dirname(PID_FILE), exist_ok=True)
        with open(PID_FILE, 'w') as f:
            f.write(str(os.getpid()))

    def _remove_pid_file(self):
        """Remove PID file"""
        try:
            os.remove(PID_FILE)
        except FileNotFoundError:
            pass

    def run(self):
        """Main run loop"""
        self._write_pid_file()
        logger.info(f"Watchdog360 Agent started. Pushing metrics every {self.interval} seconds")

        try:
            while True:
                try:
                    # Collect metrics
                    metrics = self.collector.collect_all_metrics()

                    # Push metrics
                    success = self.pusher.push_metrics(metrics)

                    if not success:
                        logger.warning("Failed to push metrics, will retry next cycle")

                except Exception as e:
                    logger.error(f"Error in metrics collection/push cycle: {e}")

                # Wait for next cycle
                time.sleep(self.interval)

        except KeyboardInterrupt:
            logger.info("Agent stopped by user")
        finally:
            self._remove_pid_file()


def main():
    """Main entry point"""
    if os.geteuid() != 0:
        logger.warning("Agent is not running as root. Some metrics may be unavailable.")

    agent = WatchdogAgent()
    agent.run()


if __name__ == "__main__":
    main()
