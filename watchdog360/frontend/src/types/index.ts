export interface Server {
  id: number
  hostname: string
  ip_address?: string
  os_type?: string
  os_version?: string
  is_active: boolean
  created_at: string
  last_seen_at?: string
  user_id: number
}

export interface ServerMetric {
  id: number
  server_id: number
  timestamp: string
  cpu_percent: number
  memory_percent: number
  disk_usage_avg?: number
  anomaly_score?: number
  is_anomaly: boolean
}

export interface Alert {
  id: number
  server_id: number
  alert_type: string
  severity: 'info' | 'warning' | 'critical'
  title: string
  message?: string
  metric_value?: number
  threshold_value?: number
  is_resolved: boolean
  is_prediction: boolean
  created_at: string
  resolved_at?: string
  predicted_at?: string
}

export interface DashboardStats {
  total_servers: number
  active_servers: number
  critical_alerts: number
  warning_alerts: number
  total_metrics_today: number
}

export interface TimeSeriesData {
  timestamps: string[]
  values: number[]
}

export interface Process {
  pid: number
  name: string
  cpu_percent: number
  memory_percent: number
  username: string
}

export interface DiskInfo {
  device: string
  mountpoint: string
  fstype: string
  total: number
  used: number
  free: number
  percent: number
}
