import axios from 'axios'
import type { Server, ServerMetric, Alert, DashboardStats, TimeSeriesData } from '../types'

const API_BASE_URL = import.meta.env.VITE_API_URL || '/api/v1'

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
})

// Add token to requests
api.interceptors.request.use((config) => {
  const token = localStorage.getItem('token')
  if (token) {
    config.headers.Authorization = `Bearer ${token}`
  }
  return config
})

// Authentication
export const authAPI = {
  login: async (email: string, password: string) => {
    const response = await api.post('/auth/login', { email, password })
    return response.data
  },
  register: async (email: string, password: string, full_name?: string) => {
    const response = await api.post('/auth/register', { email, password, full_name })
    return response.data
  },
}

// Dashboard
export const dashboardAPI = {
  getStats: async (): Promise<DashboardStats> => {
    const response = await api.get('/dashboard/stats')
    return response.data
  },
  getServers: async (): Promise<Server[]> => {
    const response = await api.get('/dashboard/servers')
    return response.data
  },
  getServerDetail: async (serverId: number) => {
    const response = await api.get(`/dashboard/servers/${serverId}`)
    return response.data
  },
  getAlerts: async (serverId?: number): Promise<Alert[]> => {
    const response = await api.get('/dashboard/alerts', {
      params: serverId ? { server_id: serverId } : {},
    })
    return response.data
  },
  resolveAlert: async (alertId: number) => {
    const response = await api.post(`/dashboard/alerts/${alertId}/resolve`)
    return response.data
  },
}

// Metrics
export const metricsAPI = {
  getLatest: async (serverId: number): Promise<ServerMetric> => {
    const response = await api.get(`/metrics/server/${serverId}/latest`)
    return response.data
  },
  getTimeSeries: async (
    serverId: number,
    metricName: string,
    hours: number = 24
  ): Promise<TimeSeriesData> => {
    const response = await api.get(`/metrics/server/${serverId}/timeseries`, {
      params: { metric_name: metricName, hours },
    })
    return response.data
  },
  getStats: async (serverId: number) => {
    const response = await api.get(`/metrics/server/${serverId}/stats`)
    return response.data
  },
}

export default api
