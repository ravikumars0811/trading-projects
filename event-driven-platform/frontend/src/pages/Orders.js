import React, { useState, useEffect } from 'react';
import { orderAPI, userAPI, productAPI } from '../services/api';

function Orders() {
  const [orders, setOrders] = useState([]);
  const [users, setUsers] = useState([]);
  const [products, setProducts] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [showForm, setShowForm] = useState(false);
  const [formData, setFormData] = useState({
    user_id: '',
    items: [{ product_id: '', quantity: 1 }],
  });

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    try {
      setLoading(true);
      const [ordersRes, usersRes, productsRes] = await Promise.all([
        orderAPI.getAll(),
        userAPI.getAll(),
        productAPI.getAll(),
      ]);
      setOrders(ordersRes.data);
      setUsers(usersRes.data);
      setProducts(productsRes.data);
      setError(null);
    } catch (err) {
      setError('Failed to load data');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      await orderAPI.create({
        user_id: parseInt(formData.user_id),
        items: formData.items.map(item => ({
          product_id: parseInt(item.product_id),
          quantity: parseInt(item.quantity),
        })),
      });
      setFormData({ user_id: '', items: [{ product_id: '', quantity: 1 }] });
      setShowForm(false);
      loadData();
    } catch (err) {
      alert('Failed to create order: ' + (err.response?.data?.detail || err.message));
    }
  };

  const addItem = () => {
    setFormData({
      ...formData,
      items: [...formData.items, { product_id: '', quantity: 1 }],
    });
  };

  const removeItem = (index) => {
    const newItems = formData.items.filter((_, i) => i !== index);
    setFormData({ ...formData, items: newItems });
  };

  const updateItem = (index, field, value) => {
    const newItems = [...formData.items];
    newItems[index][field] = value;
    setFormData({ ...formData, items: newItems });
  };

  if (loading) return <div className="loading">Loading...</div>;
  if (error) return <div className="error">{error}</div>;

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.5rem' }}>
        <h2>Orders</h2>
        <button className="btn btn-primary" onClick={() => setShowForm(!showForm)}>
          {showForm ? 'Cancel' : 'Create Order'}
        </button>
      </div>

      {showForm && (
        <div className="card">
          <h3>Create New Order</h3>
          <form onSubmit={handleSubmit}>
            <div className="form-group">
              <label>User</label>
              <select
                value={formData.user_id}
                onChange={(e) => setFormData({ ...formData, user_id: e.target.value })}
                required
              >
                <option value="">Select a user</option>
                {users.map((user) => (
                  <option key={user.id} value={user.id}>
                    {user.username} ({user.email})
                  </option>
                ))}
              </select>
            </div>

            <h4>Order Items</h4>
            {formData.items.map((item, index) => (
              <div key={index} style={{ display: 'flex', gap: '1rem', marginBottom: '1rem' }}>
                <div className="form-group" style={{ flex: 2 }}>
                  <select
                    value={item.product_id}
                    onChange={(e) => updateItem(index, 'product_id', e.target.value)}
                    required
                  >
                    <option value="">Select a product</option>
                    {products.map((product) => (
                      <option key={product.id} value={product.id}>
                        {product.name} - ${parseFloat(product.price).toFixed(2)}
                      </option>
                    ))}
                  </select>
                </div>
                <div className="form-group" style={{ flex: 1 }}>
                  <input
                    type="number"
                    min="1"
                    value={item.quantity}
                    onChange={(e) => updateItem(index, 'quantity', e.target.value)}
                    required
                  />
                </div>
                {formData.items.length > 1 && (
                  <button
                    type="button"
                    className="btn btn-danger"
                    onClick={() => removeItem(index)}
                  >
                    Remove
                  </button>
                )}
              </div>
            ))}

            <button type="button" className="btn btn-primary" onClick={addItem} style={{ marginRight: '1rem' }}>
              Add Item
            </button>
            <button type="submit" className="btn btn-success">Create Order</button>
          </form>
        </div>
      )}

      <div className="card">
        <table className="table">
          <thead>
            <tr>
              <th>Order ID</th>
              <th>User ID</th>
              <th>Total Amount</th>
              <th>Items</th>
              <th>Status</th>
              <th>Created</th>
            </tr>
          </thead>
          <tbody>
            {orders.map((order) => (
              <tr key={order.id}>
                <td>{order.id}</td>
                <td>{order.user_id}</td>
                <td>${parseFloat(order.total_amount).toFixed(2)}</td>
                <td>{order.items?.length || 0} items</td>
                <td>
                  <span className={`badge badge-${order.status === 'completed' ? 'success' : 'warning'}`}>
                    {order.status}
                  </span>
                </td>
                <td>{new Date(order.created_at).toLocaleDateString()}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default Orders;
