import { ReactNode } from 'react'

interface StatsCardProps {
  title: string
  value: number | string
  icon: ReactNode
  color: 'blue' | 'green' | 'red' | 'purple' | 'yellow'
}

const colorClasses = {
  blue: 'from-blue-500/20 to-blue-600/20 border-blue-500/50',
  green: 'from-green-500/20 to-green-600/20 border-green-500/50',
  red: 'from-red-500/20 to-red-600/20 border-red-500/50',
  purple: 'from-purple-500/20 to-purple-600/20 border-purple-500/50',
  yellow: 'from-yellow-500/20 to-yellow-600/20 border-yellow-500/50',
}

const iconColorClasses = {
  blue: 'text-blue-400',
  green: 'text-green-400',
  red: 'text-red-400',
  purple: 'text-purple-400',
  yellow: 'text-yellow-400',
}

export default function StatsCard({ title, value, icon, color }: StatsCardProps) {
  return (
    <div
      className={`bg-gradient-to-br ${colorClasses[color]} border rounded-lg p-6 backdrop-blur-sm`}
    >
      <div className="flex items-center justify-between">
        <div>
          <p className="text-sm font-medium text-gray-400">{title}</p>
          <p className="mt-2 text-3xl font-bold text-white">{value}</p>
        </div>
        <div className={`${iconColorClasses[color]}`}>{icon}</div>
      </div>
    </div>
  )
}
