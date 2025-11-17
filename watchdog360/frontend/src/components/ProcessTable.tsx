import { Process } from '../types'

interface ProcessTableProps {
  processes: Process[]
}

export default function ProcessTable({ processes }: ProcessTableProps) {
  if (!processes || processes.length === 0) {
    return (
      <div className="glass rounded-lg p-6 text-center text-gray-500">
        No process data available
      </div>
    )
  }

  return (
    <div className="glass rounded-lg overflow-hidden">
      <div className="overflow-x-auto">
        <table className="w-full">
          <thead className="bg-gray-800/50">
            <tr>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-400 uppercase tracking-wider">
                PID
              </th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-400 uppercase tracking-wider">
                Name
              </th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-400 uppercase tracking-wider">
                User
              </th>
              <th className="px-6 py-3 text-right text-xs font-medium text-gray-400 uppercase tracking-wider">
                CPU %
              </th>
              <th className="px-6 py-3 text-right text-xs font-medium text-gray-400 uppercase tracking-wider">
                Memory %
              </th>
            </tr>
          </thead>
          <tbody className="divide-y divide-gray-700">
            {processes.map((process, index) => (
              <tr key={`${process.pid}-${index}`} className="hover:bg-white/5">
                <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-300">
                  {process.pid}
                </td>
                <td className="px-6 py-4 whitespace-nowrap text-sm text-white font-medium">
                  {process.name}
                </td>
                <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-400">
                  {process.username}
                </td>
                <td className="px-6 py-4 whitespace-nowrap text-sm text-right">
                  <span
                    className={
                      process.cpu_percent > 50
                        ? 'text-red-400'
                        : process.cpu_percent > 20
                        ? 'text-yellow-400'
                        : 'text-green-400'
                    }
                  >
                    {process.cpu_percent.toFixed(1)}%
                  </span>
                </td>
                <td className="px-6 py-4 whitespace-nowrap text-sm text-right">
                  <span
                    className={
                      process.memory_percent > 50
                        ? 'text-red-400'
                        : process.memory_percent > 20
                        ? 'text-yellow-400'
                        : 'text-green-400'
                    }
                  >
                    {process.memory_percent.toFixed(1)}%
                  </span>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  )
}
