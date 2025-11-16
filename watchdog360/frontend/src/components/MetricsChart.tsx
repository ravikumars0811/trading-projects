import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts'
import { format } from 'date-fns'
import { TimeSeriesData } from '../types'

interface MetricsChartProps {
  title: string
  data: TimeSeriesData | null
  color: string
  unit?: string
}

export default function MetricsChart({ title, data, color, unit = '' }: MetricsChartProps) {
  if (!data || !data.timestamps || data.timestamps.length === 0) {
    return (
      <div className="glass rounded-lg p-6">
        <h3 className="text-lg font-semibold text-white mb-4">{title}</h3>
        <div className="h-64 flex items-center justify-center text-gray-500">
          No data available
        </div>
      </div>
    )
  }

  // Transform data for Recharts
  const chartData = data.timestamps.map((timestamp, index) => ({
    timestamp: new Date(timestamp).getTime(),
    value: data.values[index],
  }))

  return (
    <div className="glass rounded-lg p-6">
      <h3 className="text-lg font-semibold text-white mb-4">{title}</h3>
      <ResponsiveContainer width="100%" height={300}>
        <LineChart data={chartData}>
          <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
          <XAxis
            dataKey="timestamp"
            stroke="#9ca3af"
            tickFormatter={(timestamp) => format(new Date(timestamp), 'HH:mm')}
          />
          <YAxis stroke="#9ca3af" domain={[0, 100]} />
          <Tooltip
            contentStyle={{
              backgroundColor: '#1f2937',
              border: '1px solid #374151',
              borderRadius: '8px',
            }}
            labelStyle={{ color: '#9ca3af' }}
            itemStyle={{ color: '#fff' }}
            labelFormatter={(timestamp) => format(new Date(timestamp), 'HH:mm:ss')}
            formatter={(value: number) => [`${value.toFixed(2)}${unit}`, title]}
          />
          <Line
            type="monotone"
            dataKey="value"
            stroke={color}
            strokeWidth={2}
            dot={false}
            activeDot={{ r: 6 }}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  )
}
