import { Server, Activity, AlertCircle } from 'lucide-react'
import { Server as ServerType } from '../types'
import { formatDistanceToNow } from 'date-fns'

interface ServerCardProps {
  server: ServerType
  onClick: () => void
}

export default function ServerCard({ server, onClick }: ServerCardProps) {
  const isOnline = server.last_seen_at
    ? new Date().getTime() - new Date(server.last_seen_at).getTime() < 60000
    : false

  return (
    <div
      onClick={onClick}
      className="glass rounded-lg p-6 cursor-pointer hover:bg-white/10 transition-all border border-gray-700 hover:border-blue-500"
    >
      <div className="flex items-start justify-between mb-4">
        <div className="flex items-center gap-3">
          <div className="p-2 bg-blue-500/20 rounded-lg">
            <Server className="w-6 h-6 text-blue-400" />
          </div>
          <div>
            <h3 className="font-semibold text-white">{server.hostname}</h3>
            <p className="text-sm text-gray-400">{server.ip_address || 'N/A'}</p>
          </div>
        </div>
        <div className="flex items-center gap-2">
          {isOnline ? (
            <Activity className="w-5 h-5 text-green-400" />
          ) : (
            <AlertCircle className="w-5 h-5 text-red-400" />
          )}
        </div>
      </div>

      <div className="space-y-2">
        <div className="flex justify-between text-sm">
          <span className="text-gray-400">Status</span>
          <span className={isOnline ? 'text-green-400' : 'text-red-400'}>
            {isOnline ? 'Online' : 'Offline'}
          </span>
        </div>

        {server.last_seen_at && (
          <div className="flex justify-between text-sm">
            <span className="text-gray-400">Last Seen</span>
            <span className="text-gray-300">
              {formatDistanceToNow(new Date(server.last_seen_at), { addSuffix: true })}
            </span>
          </div>
        )}

        {server.os_type && (
          <div className="flex justify-between text-sm">
            <span className="text-gray-400">OS</span>
            <span className="text-gray-300">{server.os_type}</span>
          </div>
        )}
      </div>
    </div>
  )
}
