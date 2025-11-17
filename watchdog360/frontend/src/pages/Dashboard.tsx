import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useStore } from '../store/useStore'
import { dashboardAPI } from '../services/api'
import { Server as ServerIcon, Activity, AlertTriangle, TrendingUp, LogOut } from 'lucide-react'
import ServerCard from '../components/ServerCard'
import AlertList from '../components/AlertList'
import StatsCard from '../components/StatsCard'

export default function Dashboard() {
  const navigate = useNavigate()
  const { stats, servers, alerts, setStats, setServers, setAlerts, logout } = useStore()
  const [loading, setLoading] = useState(true)
  const [refreshInterval, setRefreshInterval] = useState<number | null>(null)

  const loadDashboard = async () => {
    try {
      const [statsData, serversData, alertsData] = await Promise.all([
        dashboardAPI.getStats(),
        dashboardAPI.getServers(),
        dashboardAPI.getAlerts(),
      ])

      setStats(statsData)
      setServers(serversData)
      setAlerts(alertsData)
    } catch (err) {
      console.error('Failed to load dashboard:', err)
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    loadDashboard()

    // Refresh every 10 seconds
    const interval = window.setInterval(() => {
      loadDashboard()
    }, 10000)

    setRefreshInterval(interval)

    return () => {
      if (interval) clearInterval(interval)
    }
  }, [])

  const handleLogout = () => {
    logout()
    navigate('/login')
  }

  if (loading) {
    return (
      <div className="min-h-screen bg-gray-900 flex items-center justify-center">
        <div className="text-white text-xl">Loading dashboard...</div>
      </div>
    )
  }

  return (
    <div className="min-h-screen bg-gray-900">
      {/* Header */}
      <header className="bg-gray-800 border-b border-gray-700">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
          <div className="flex items-center justify-between">
            <div>
              <h1 className="text-2xl font-bold bg-gradient-to-r from-blue-400 to-purple-500 bg-clip-text text-transparent">
                Watchdog360
              </h1>
              <p className="text-sm text-gray-400 mt-1">Server Performance Monitoring</p>
            </div>
            <button
              onClick={handleLogout}
              className="flex items-center gap-2 px-4 py-2 text-gray-300 hover:text-white hover:bg-gray-700 rounded-lg transition-colors"
            >
              <LogOut className="w-5 h-5" />
              Logout
            </button>
          </div>
        </div>
      </header>

      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {/* Stats Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
          <StatsCard
            title="Total Servers"
            value={stats?.total_servers || 0}
            icon={<ServerIcon className="w-6 h-6" />}
            color="blue"
          />
          <StatsCard
            title="Active Servers"
            value={stats?.active_servers || 0}
            icon={<Activity className="w-6 h-6" />}
            color="green"
          />
          <StatsCard
            title="Critical Alerts"
            value={stats?.critical_alerts || 0}
            icon={<AlertTriangle className="w-6 h-6" />}
            color="red"
          />
          <StatsCard
            title="Metrics Today"
            value={stats?.total_metrics_today || 0}
            icon={<TrendingUp className="w-6 h-6" />}
            color="purple"
          />
        </div>

        {/* Alerts Section */}
        {alerts && alerts.length > 0 && (
          <div className="mb-8">
            <h2 className="text-xl font-semibold text-white mb-4">Active Alerts</h2>
            <AlertList alerts={alerts} />
          </div>
        )}

        {/* Servers Grid */}
        <div>
          <h2 className="text-xl font-semibold text-white mb-4">Servers</h2>
          {servers.length === 0 ? (
            <div className="glass rounded-lg p-8 text-center">
              <ServerIcon className="w-16 h-16 text-gray-500 mx-auto mb-4" />
              <h3 className="text-lg font-medium text-gray-300 mb-2">No Servers Yet</h3>
              <p className="text-gray-400 mb-4">
                Install the Watchdog360 agent on your servers to start monitoring
              </p>
              <div className="bg-gray-800 rounded-lg p-4 text-left">
                <code className="text-sm text-blue-400">
                  curl -s https://watchdog360.com/install.sh | bash
                </code>
              </div>
            </div>
          ) : (
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
              {servers.map((server) => (
                <ServerCard
                  key={server.id}
                  server={server}
                  onClick={() => navigate(`/server/${server.id}`)}
                />
              ))}
            </div>
          )}
        </div>
      </main>
    </div>
  )
}
