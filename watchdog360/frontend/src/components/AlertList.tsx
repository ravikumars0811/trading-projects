import { AlertTriangle, Info, XCircle, X } from 'lucide-react'
import { Alert } from '../types'
import { formatDistanceToNow } from 'date-fns'
import { dashboardAPI } from '../services/api'

interface AlertListProps {
  alerts: Alert[]
  onResolve?: () => void
}

const severityConfig = {
  critical: {
    icon: XCircle,
    color: 'text-red-400',
    bg: 'bg-red-500/10',
    border: 'border-red-500/50',
  },
  warning: {
    icon: AlertTriangle,
    color: 'text-yellow-400',
    bg: 'bg-yellow-500/10',
    border: 'border-yellow-500/50',
  },
  info: {
    icon: Info,
    color: 'text-blue-400',
    bg: 'bg-blue-500/10',
    border: 'border-blue-500/50',
  },
}

export default function AlertList({ alerts, onResolve }: AlertListProps) {
  const handleResolve = async (alertId: number) => {
    try {
      await dashboardAPI.resolveAlert(alertId)
      if (onResolve) onResolve()
    } catch (err) {
      console.error('Failed to resolve alert:', err)
    }
  }

  return (
    <div className="space-y-3">
      {alerts.map((alert) => {
        const config = severityConfig[alert.severity]
        const Icon = config.icon

        return (
          <div
            key={alert.id}
            className={`${config.bg} ${config.border} border rounded-lg p-4`}
          >
            <div className="flex items-start justify-between">
              <div className="flex items-start gap-3 flex-1">
                <Icon className={`w-5 h-5 ${config.color} mt-0.5`} />
                <div className="flex-1">
                  <div className="flex items-center gap-2 mb-1">
                    <h4 className="font-medium text-white">{alert.title}</h4>
                    {alert.is_prediction && (
                      <span className="text-xs px-2 py-0.5 bg-purple-500/20 text-purple-400 rounded">
                        AI Prediction
                      </span>
                    )}
                  </div>
                  {alert.message && (
                    <p className="text-sm text-gray-400 mb-2">{alert.message}</p>
                  )}
                  <div className="flex items-center gap-4 text-xs text-gray-500">
                    <span>{formatDistanceToNow(new Date(alert.created_at), { addSuffix: true })}</span>
                    <span className="capitalize">{alert.alert_type}</span>
                  </div>
                </div>
              </div>

              <button
                onClick={() => handleResolve(alert.id)}
                className="text-gray-400 hover:text-white ml-2"
                title="Resolve alert"
              >
                <X className="w-5 h-5" />
              </button>
            </div>
          </div>
        )
      })}
    </div>
  )
}
