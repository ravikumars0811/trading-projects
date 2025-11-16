'use client';

import { useState, useEffect } from 'react';
import { useRouter } from 'next/navigation';
import { useAuthStore } from '@/lib/store';
import { endpointsAPI } from '@/lib/api';
import { Endpoint, EndpointStats } from '@/types';
import toast from 'react-hot-toast';
import EndpointCard from '@/components/EndpointCard';
import AddEndpointModal from '@/components/AddEndpointModal';

export default function Dashboard() {
  const router = useRouter();
  const { token, user, logout } = useAuthStore();
  const [endpoints, setEndpoints] = useState<Endpoint[]>([]);
  const [stats, setStats] = useState<Record<number, EndpointStats>>({});
  const [loading, setLoading] = useState(true);
  const [showAddModal, setShowAddModal] = useState(false);

  useEffect(() => {
    if (!token) {
      router.push('/login');
      return;
    }
    loadEndpoints();
  }, [token, router]);

  const loadEndpoints = async () => {
    try {
      const response = await endpointsAPI.list();
      const endpointsList = response.data;
      setEndpoints(endpointsList);

      // Load stats for each endpoint
      const statsPromises = endpointsList.map(async (endpoint: Endpoint) => {
        try {
          const statsResponse = await endpointsAPI.stats(endpoint.id, '24h');
          return { id: endpoint.id, stats: statsResponse.data };
        } catch (error) {
          return { id: endpoint.id, stats: null };
        }
      });

      const statsResults = await Promise.all(statsPromises);
      const statsMap: Record<number, EndpointStats> = {};
      statsResults.forEach(({ id, stats }) => {
        if (stats) statsMap[id] = stats;
      });
      setStats(statsMap);
    } catch (error: any) {
      toast.error('Failed to load endpoints');
    } finally {
      setLoading(false);
    }
  };

  const handleAddEndpoint = async (data: any) => {
    try {
      await endpointsAPI.create(data);
      toast.success('Endpoint added successfully');
      setShowAddModal(false);
      loadEndpoints();
    } catch (error: any) {
      toast.error(error.response?.data?.detail || 'Failed to add endpoint');
    }
  };

  const handleDeleteEndpoint = async (id: number) => {
    if (!confirm('Are you sure you want to delete this endpoint?')) return;

    try {
      await endpointsAPI.delete(id);
      toast.success('Endpoint deleted');
      loadEndpoints();
    } catch (error: any) {
      toast.error('Failed to delete endpoint');
    }
  };

  const handleCheckNow = async (id: number) => {
    try {
      await endpointsAPI.check(id);
      toast.success('Health check initiated');
      setTimeout(() => loadEndpoints(), 3000);
    } catch (error: any) {
      toast.error('Failed to initiate check');
    }
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center min-h-screen">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-primary-600"></div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900">
      {/* Header */}
      <header className="bg-white dark:bg-gray-800 shadow">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
          <div className="flex justify-between items-center">
            <div>
              <h1 className="text-2xl font-bold text-gray-900 dark:text-white">
                API Uptime Monitor
              </h1>
              <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
                Welcome, {user?.username}
              </p>
            </div>
            <div className="flex gap-4">
              <button
                onClick={() => router.push('/dashboard/alerts')}
                className="px-4 py-2 text-sm font-medium text-gray-700 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg"
              >
                Alerts
              </button>
              <button
                onClick={logout}
                className="px-4 py-2 text-sm font-medium text-gray-700 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg"
              >
                Logout
              </button>
            </div>
          </div>
        </div>
      </header>

      {/* Main Content */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {/* Stats Overview */}
        <div className="grid grid-cols-1 md:grid-cols-4 gap-6 mb-8">
          <div className="bg-white dark:bg-gray-800 rounded-lg shadow p-6">
            <div className="text-sm font-medium text-gray-600 dark:text-gray-400">
              Total Endpoints
            </div>
            <div className="text-3xl font-bold text-gray-900 dark:text-white mt-2">
              {endpoints.length}
            </div>
          </div>

          <div className="bg-white dark:bg-gray-800 rounded-lg shadow p-6">
            <div className="text-sm font-medium text-gray-600 dark:text-gray-400">
              Online
            </div>
            <div className="text-3xl font-bold text-green-600 mt-2">
              {Object.values(stats).filter((s) => s.current_status === 'up').length}
            </div>
          </div>

          <div className="bg-white dark:bg-gray-800 rounded-lg shadow p-6">
            <div className="text-sm font-medium text-gray-600 dark:text-gray-400">
              Degraded
            </div>
            <div className="text-3xl font-bold text-yellow-600 mt-2">
              {Object.values(stats).filter((s) => s.current_status === 'degraded').length}
            </div>
          </div>

          <div className="bg-white dark:bg-gray-800 rounded-lg shadow p-6">
            <div className="text-sm font-medium text-gray-600 dark:text-gray-400">
              Down
            </div>
            <div className="text-3xl font-bold text-red-600 mt-2">
              {Object.values(stats).filter((s) => s.current_status === 'down').length}
            </div>
          </div>
        </div>

        {/* Endpoints List */}
        <div className="flex justify-between items-center mb-6">
          <h2 className="text-xl font-semibold text-gray-900 dark:text-white">
            Your Endpoints
          </h2>
          <button
            onClick={() => setShowAddModal(true)}
            className="bg-primary-600 hover:bg-primary-700 text-white px-4 py-2 rounded-lg font-medium"
          >
            + Add Endpoint
          </button>
        </div>

        {endpoints.length === 0 ? (
          <div className="bg-white dark:bg-gray-800 rounded-lg shadow p-12 text-center">
            <p className="text-gray-600 dark:text-gray-400 mb-4">
              No endpoints yet. Add your first endpoint to start monitoring!
            </p>
            <button
              onClick={() => setShowAddModal(true)}
              className="bg-primary-600 hover:bg-primary-700 text-white px-6 py-3 rounded-lg font-medium"
            >
              Add Your First Endpoint
            </button>
          </div>
        ) : (
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            {endpoints.map((endpoint) => (
              <EndpointCard
                key={endpoint.id}
                endpoint={endpoint}
                stats={stats[endpoint.id]}
                onDelete={() => handleDeleteEndpoint(endpoint.id)}
                onCheckNow={() => handleCheckNow(endpoint.id)}
                onViewDetails={() => router.push(`/dashboard/endpoint/${endpoint.id}`)}
              />
            ))}
          </div>
        )}
      </main>

      {/* Add Endpoint Modal */}
      {showAddModal && (
        <AddEndpointModal
          onClose={() => setShowAddModal(false)}
          onSubmit={handleAddEndpoint}
        />
      )}
    </div>
  );
}
