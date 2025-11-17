import { useEffect, useState } from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import { ArrowLeft, Activity, Cpu, HardDrive, Network } from 'lucide-react'
import { dashboardAPI, metricsAPI } from '../services/api'
import MetricsChart from '../components/MetricsChart'
import ProcessTable from '../components/ProcessTable'
import AlertList from '../components/AlertList'

export default function ServerDetail() {
  const { id } = useParams<{ id: string }>()
  const navigate = useNavigate()
  const [serverData, setServerData] = useState<any>(null)
  const [cpuData, setCpuData] = useState<any>(null)
  const [memoryData, setMemoryData] = useState<any>(null)
  const [alerts, setAlerts] = useState<any[]>([])
  const [loading, setLoading] = useState(true)

  const loadServerData = async () => {
    if (!id) return

    try {
      const [detail, cpu, memory, alertsData] = await Promise.all([
        dashboardAPI.getServerDetail(parseInt(id)),
        metricsAPI.getTimeSeries(parseInt(id), 'cpu_percent', 1),
        metricsAPI.getTimeSeries(parseInt(id), 'memory_percent', 1),
        dashboardAPI.getAlerts(parseInt(id)),
      ])

      setServerData(detail)
      setCpuData(cpu)
      setMemoryData(memory)
      setAlerts(alertsData)
    } catch (err) {
      console.error('Failed to load server data:', err)
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    loadServerData()

    // Refresh every 10 seconds
    const interval = setInterval(() => {
      loadServerData()
    }, 10000)

    return () => clearInterval(interval)
  }, [id])

  if (loading) {
    return (
      <div className="min-h-screen bg-gray-900 flex items-center justify-center">
        <div className="text-white text-xl">Loading server details...</div>
      </div>
    )
  }

  if (!serverData) {
    return (
      <div className="min-h-screen bg-gray-900 flex items-center justify-center">
        <div className="text-white text-xl">Server not found</div>
      </div>
    )
  }

  const { server, latest_metrics } = serverData

  return (
    <div className="min-h-screen bg-gray-900">
      {/* Header */}
      <header className="bg-gray-800 border-b border-gray-700">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
          <div className="flex items-center gap-4">
            <button
              onClick={() => navigate('/dashboard')}
              className="p-2 hover:bg-gray-700 rounded-lg transition-colors"
            >
              <ArrowLeft className="w-5 h-5 text-gray-400" />
            </button>
            <div>
              <h1 className="text-2xl font-bold text-white">{server.hostname}</h1>
              <p className="text-sm text-gray-400">{server.ip_address}</p>
            </div>
          </div>
        </div>
      </header>

      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {/* Current Stats */}
        {latest_metrics && (
          <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
            <div className="glass rounded-lg p-6">
              <div className="flex items-center gap-3 mb-2">
                <Cpu className="w-5 h-5 text-blue-400" />
                <h3 className="font-semibold text-white">CPU Usage</h3>
              </div>
              <p className="text-3xl font-bold text-white">
                {latest_metrics.cpu_percent.toFixed(1)}%
              </p>
              {latest_metrics.is_anomaly && (
                <span className="inline-block mt-2 text-xs px-2 py-1 bg-red-500/20 text-red-400 rounded">
                  Anomaly Detected
                </span>
              )}
            </div>

            <div className="glass rounded-lg p-6">
              <div className="flex items-center gap-3 mb-2">
                <Activity className="w-5 h-5 text-green-400" />
                <h3 className="font-semibold text-white">Memory Usage</h3>
              </div>
              <p className="text-3xl font-bold text-white">
                {latest_metrics.memory_percent.toFixed(1)}%
              </p>
            </div>

            <div className="glass rounded-lg p-6">
              <div className="flex items-center gap-3 mb-2">
                <HardDrive className="w-5 h-5 text-purple-400" />
                <h3 className="font-semibold text-white">Disk Usage</h3>
              </div>
              <p className="text-3xl font-bold text-white">
                {latest_metrics.disk_usage_avg?.toFixed(1) || 'N/A'}%
              </p>
            </div>
          </div>
        )}

        {/* Alerts */}
        {alerts.length > 0 && (
          <div className="mb-8">
            <h2 className="text-xl font-semibold text-white mb-4">Active Alerts</h2>
            <AlertList alerts={alerts} onResolve={loadServerData} />
          </div>
        )}

        {/* Charts */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-8">
          <MetricsChart
            title="CPU Usage"
            data={cpuData}
            color="#3b82f6"
            unit="%"
          />
          <MetricsChart
            title="Memory Usage"
            data={memoryData}
            color="#10b981"
            unit="%"
          />
        </div>

        {/* Processes Table */}
        {latest_metrics && latest_metrics.top_processes && (
          <div>
            <h2 className="text-xl font-semibold text-white mb-4">Top Processes</h2>
            <ProcessTable processes={latest_metrics.top_processes} />
          </div>
        )}
      </main>
    </div>
  )
}
