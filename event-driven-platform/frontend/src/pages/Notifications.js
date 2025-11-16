import React, { useState, useEffect } from 'react';
import { notificationAPI } from '../services/api';

function Notifications() {
  const [notifications, setNotifications] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    loadNotifications();
    const interval = setInterval(loadNotifications, 5000); // Refresh every 5 seconds
    return () => clearInterval(interval);
  }, []);

  const loadNotifications = async () => {
    try {
      setLoading(true);
      const response = await notificationAPI.getAll();
      setNotifications(response.data.notifications);
      setError(null);
    } catch (err) {
      setError('Failed to load notifications');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  if (loading && notifications.length === 0) return <div className="loading">Loading...</div>;
  if (error) return <div className="error">{error}</div>;

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.5rem' }}>
        <h2>Notifications</h2>
        <button className="btn btn-primary" onClick={loadNotifications}>
          Refresh
        </button>
      </div>

      {notifications.length === 0 ? (
        <div className="card">
          <p>No notifications yet. Create users, products, or orders to see event notifications here.</p>
        </div>
      ) : (
        <div>
          {notifications.map((notification, index) => (
            <div key={index} className="card">
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'start' }}>
                <div style={{ flex: 1 }}>
                  <span className={`badge badge-info`} style={{ marginBottom: '0.5rem' }}>
                    {notification.type}
                  </span>
                  <p style={{ margin: '0.5rem 0' }}>{notification.message}</p>
                  {notification.user_id && (
                    <p style={{ margin: '0', color: '#666', fontSize: '0.9rem' }}>
                      User ID: {notification.user_id}
                    </p>
                  )}
                </div>
                {notification.timestamp && (
                  <div style={{ color: '#999', fontSize: '0.875rem' }}>
                    {new Date(notification.timestamp * 1000).toLocaleString()}
                  </div>
                )}
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

export default Notifications;
