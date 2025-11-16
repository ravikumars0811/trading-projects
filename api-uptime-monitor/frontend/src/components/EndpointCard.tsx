import { Endpoint, EndpointStats } from '@/types';
import { formatDistanceToNow } from 'date-fns';

interface EndpointCardProps {
  endpoint: Endpoint;
  stats?: EndpointStats;
  onDelete: () => void;
  onCheckNow: () => void;
  onViewDetails: () => void;
}

export default function EndpointCard({
  endpoint,
  stats,
  onDelete,
  onCheckNow,
  onViewDetails,
}: EndpointCardProps) {
  const getStatusColor = (status?: string) => {
    switch (status) {
      case 'up':
        return 'bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-200';
      case 'down':
        return 'bg-red-100 text-red-800 dark:bg-red-900 dark:text-red-200';
      case 'degraded':
        return 'bg-yellow-100 text-yellow-800 dark:bg-yellow-900 dark:text-yellow-200';
      default:
        return 'bg-gray-100 text-gray-800 dark:bg-gray-700 dark:text-gray-200';
    }
  };

  const getStatusDot = (status?: string) => {
    switch (status) {
      case 'up':
        return 'bg-green-500';
      case 'down':
        return 'bg-red-500';
      case 'degraded':
        return 'bg-yellow-500';
      default:
        return 'bg-gray-500';
    }
  };

  return (
    <div className="bg-white dark:bg-gray-800 rounded-lg shadow hover:shadow-lg transition-shadow p-6">
      {/* Header */}
      <div className="flex justify-between items-start mb-4">
        <div className="flex-1">
          <div className="flex items-center gap-3 mb-2">
            <div className={`w-3 h-3 rounded-full ${getStatusDot(stats?.current_status)}`} />
            <h3 className="text-lg font-semibold text-gray-900 dark:text-white">
              {endpoint.name}
            </h3>
            <span
              className={`px-2 py-1 text-xs font-medium rounded-full ${getStatusColor(
                stats?.current_status
              )}`}
            >
              {stats?.current_status?.toUpperCase() || 'UNKNOWN'}
            </span>
          </div>
          <p className="text-sm text-gray-600 dark:text-gray-400 truncate">
            {endpoint.method} {endpoint.url}
          </p>
        </div>
      </div>

      {/* Stats */}
      {stats && (
        <div className="grid grid-cols-3 gap-4 mb-4">
          <div>
            <div className="text-xs text-gray-600 dark:text-gray-400">Uptime</div>
            <div className="text-lg font-semibold text-gray-900 dark:text-white">
              {stats.uptime_percentage.toFixed(2)}%
            </div>
          </div>
          <div>
            <div className="text-xs text-gray-600 dark:text-gray-400">Avg Response</div>
            <div className="text-lg font-semibold text-gray-900 dark:text-white">
              {stats.avg_response_time.toFixed(0)}ms
            </div>
          </div>
          <div>
            <div className="text-xs text-gray-600 dark:text-gray-400">Failures</div>
            <div className="text-lg font-semibold text-gray-900 dark:text-white">
              {stats.failed_checks}
            </div>
          </div>
        </div>
      )}

      {/* Last Check */}
      {endpoint.last_checked_at && (
        <div className="text-xs text-gray-500 dark:text-gray-400 mb-4">
          Last checked {formatDistanceToNow(new Date(endpoint.last_checked_at))} ago
        </div>
      )}

      {/* Actions */}
      <div className="flex gap-2 border-t border-gray-200 dark:border-gray-700 pt-4">
        <button
          onClick={onViewDetails}
          className="flex-1 bg-primary-600 hover:bg-primary-700 text-white text-sm font-medium py-2 px-4 rounded"
        >
          View Details
        </button>
        <button
          onClick={onCheckNow}
          className="bg-gray-100 hover:bg-gray-200 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-700 dark:text-gray-300 text-sm font-medium py-2 px-4 rounded"
        >
          Check Now
        </button>
        <button
          onClick={onDelete}
          className="bg-red-100 hover:bg-red-200 dark:bg-red-900 dark:hover:bg-red-800 text-red-700 dark:text-red-200 text-sm font-medium py-2 px-4 rounded"
        >
          Delete
        </button>
      </div>
    </div>
  );
}
