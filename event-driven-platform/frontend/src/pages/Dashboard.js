import React, { useState, useEffect } from 'react';
import { userAPI, productAPI, orderAPI, notificationAPI } from '../services/api';

function Dashboard() {
  const [stats, setStats] = useState({
    users: 0,
    products: 0,
    orders: 0,
    notifications: 0,
  });
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    loadDashboardData();
  }, []);

  const loadDashboardData = async () => {
    try {
      setLoading(true);
      const [users, products, orders, notifications] = await Promise.all([
        userAPI.getAll(),
        productAPI.getAll(),
        orderAPI.getAll(),
        notificationAPI.getAll(),
      ]);

      setStats({
        users: users.data.length,
        products: products.data.length,
        orders: orders.data.length,
        notifications: notifications.data.count,
      });
      setError(null);
    } catch (err) {
      setError('Failed to load dashboard data');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  if (loading) return <div className="loading">Loading...</div>;
  if (error) return <div className="error">{error}</div>;

  return (
    <div>
      <h2>Dashboard</h2>
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))', gap: '1.5rem', marginTop: '2rem' }}>
        <div className="card" style={{ textAlign: 'center' }}>
          <h3 style={{ color: '#3498db', fontSize: '2.5rem', margin: '0' }}>{stats.users}</h3>
          <p style={{ color: '#666', marginTop: '0.5rem' }}>Total Users</p>
        </div>
        <div className="card" style={{ textAlign: 'center' }}>
          <h3 style={{ color: '#2ecc71', fontSize: '2.5rem', margin: '0' }}>{stats.products}</h3>
          <p style={{ color: '#666', marginTop: '0.5rem' }}>Total Products</p>
        </div>
        <div className="card" style={{ textAlign: 'center' }}>
          <h3 style={{ color: '#e67e22', fontSize: '2.5rem', margin: '0' }}>{stats.orders}</h3>
          <p style={{ color: '#666', marginTop: '0.5rem' }}>Total Orders</p>
        </div>
        <div className="card" style={{ textAlign: 'center' }}>
          <h3 style={{ color: '#9b59b6', fontSize: '2.5rem', margin: '0' }}>{stats.notifications}</h3>
          <p style={{ color: '#666', marginTop: '0.5rem' }}>Notifications</p>
        </div>
      </div>

      <div className="card" style={{ marginTop: '2rem' }}>
        <h3>System Information</h3>
        <p><strong>Architecture:</strong> Event-Driven Microservices</p>
        <p><strong>Services:</strong> User, Product, Order, Inventory, Notification</p>
        <p><strong>Technologies:</strong> FastAPI, PostgreSQL, MongoDB, Redis, Kafka</p>
        <p><strong>Monitoring:</strong> Prometheus & Grafana</p>
        <p><strong>Logging:</strong> ELK Stack</p>
      </div>
    </div>
  );
}

export default Dashboard;
