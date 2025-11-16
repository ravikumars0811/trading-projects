export interface User {
  id: number;
  email: string;
  username: string;
  full_name?: string;
  is_active: boolean;
  created_at: string;
}

export interface Endpoint {
  id: number;
  user_id: number;
  name: string;
  url: string;
  method: 'GET' | 'POST' | 'PUT' | 'DELETE' | 'PATCH' | 'HEAD' | 'OPTIONS';
  headers?: Record<string, string>;
  body?: any;
  expected_status_codes: number[];
  timeout: number;
  check_interval: number;
  is_active: boolean;
  verify_ssl: boolean;
  follow_redirects: boolean;
  response_time_threshold: number;
  check_ssl_expiry: boolean;
  ssl_expiry_alert_days: number;
  created_at: string;
  updated_at: string;
  last_checked_at?: string;
}

export interface HealthCheck {
  id: number;
  endpoint_id: number;
  status_code?: number;
  response_time?: number;
  is_success: boolean;
  error_message?: string;
  ssl_expiry_date?: string;
  ssl_days_remaining?: number;
  is_anomaly: boolean;
  anomaly_score?: number;
  checked_at: string;
}

export interface HealthCheckStats {
  period: string;
  uptime_percentage: number;
  avg_response_time: number;
  min_response_time: number;
  max_response_time: number;
  total_checks: number;
  successful_checks: number;
  failed_checks: number;
  anomalies_detected: number;
}

export interface EndpointStats {
  endpoint_id: number;
  uptime_percentage: number;
  avg_response_time: number;
  total_checks: number;
  failed_checks: number;
  last_check?: string;
  current_status: 'up' | 'down' | 'degraded' | 'unknown';
}

export interface AlertConfig {
  id: number;
  user_id: number;
  channel: 'email' | 'telegram' | 'slack';
  alert_types: string[];
  is_active: boolean;
  email?: string;
  telegram_chat_id?: string;
  slack_webhook_url?: string;
  cooldown_minutes: number;
  created_at: string;
  updated_at: string;
}
