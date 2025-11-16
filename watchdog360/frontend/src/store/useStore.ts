import { create } from 'zustand'
import type { Server, Alert, DashboardStats } from '../types'

interface AppState {
  // Auth
  token: string | null
  user: any | null
  setAuth: (token: string, user: any) => void
  logout: () => void

  // Dashboard
  stats: DashboardStats | null
  servers: Server[]
  alerts: Alert[]
  selectedServer: Server | null
  setStats: (stats: DashboardStats) => void
  setServers: (servers: Server[]) => void
  setAlerts: (alerts: Alert[]) => void
  setSelectedServer: (server: Server | null) => void
}

export const useStore = create<AppState>((set) => ({
  // Auth
  token: localStorage.getItem('token'),
  user: null,
  setAuth: (token, user) => {
    localStorage.setItem('token', token)
    set({ token, user })
  },
  logout: () => {
    localStorage.removeItem('token')
    set({ token: null, user: null })
  },

  // Dashboard
  stats: null,
  servers: [],
  alerts: [],
  selectedServer: null,
  setStats: (stats) => set({ stats }),
  setServers: (servers) => set({ servers }),
  setAlerts: (alerts) => set({ alerts }),
  setSelectedServer: (selectedServer) => set({ selectedServer }),
}))
