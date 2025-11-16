import axios from 'axios';

const USER_SERVICE_URL = process.env.REACT_APP_USER_SERVICE_URL || 'http://localhost:8001';
const PRODUCT_SERVICE_URL = process.env.REACT_APP_PRODUCT_SERVICE_URL || 'http://localhost:8002';
const ORDER_SERVICE_URL = process.env.REACT_APP_ORDER_SERVICE_URL || 'http://localhost:8003';
const INVENTORY_SERVICE_URL = process.env.REACT_APP_INVENTORY_SERVICE_URL || 'http://localhost:8004';
const NOTIFICATION_SERVICE_URL = process.env.REACT_APP_NOTIFICATION_SERVICE_URL || 'http://localhost:8005';

// User Service APIs
export const userAPI = {
  getAll: () => axios.get(`${USER_SERVICE_URL}/users`),
  getById: (id) => axios.get(`${USER_SERVICE_URL}/users/${id}`),
  create: (data) => axios.post(`${USER_SERVICE_URL}/users`, data),
  update: (id, data) => axios.put(`${USER_SERVICE_URL}/users/${id}`, data),
  delete: (id) => axios.delete(`${USER_SERVICE_URL}/users/${id}`),
};

// Product Service APIs
export const productAPI = {
  getAll: (category = null) => {
    const url = category
      ? `${PRODUCT_SERVICE_URL}/products?category=${category}`
      : `${PRODUCT_SERVICE_URL}/products`;
    return axios.get(url);
  },
  getById: (id) => axios.get(`${PRODUCT_SERVICE_URL}/products/${id}`),
  create: (data) => axios.post(`${PRODUCT_SERVICE_URL}/products`, data),
  update: (id, data) => axios.put(`${PRODUCT_SERVICE_URL}/products/${id}`, data),
  delete: (id) => axios.delete(`${PRODUCT_SERVICE_URL}/products/${id}`),
};

// Order Service APIs
export const orderAPI = {
  getAll: (userId = null) => {
    const url = userId
      ? `${ORDER_SERVICE_URL}/orders?user_id=${userId}`
      : `${ORDER_SERVICE_URL}/orders`;
    return axios.get(url);
  },
  getById: (id) => axios.get(`${ORDER_SERVICE_URL}/orders/${id}`),
  create: (data) => axios.post(`${ORDER_SERVICE_URL}/orders`, data),
  updateStatus: (id, status) => axios.put(`${ORDER_SERVICE_URL}/orders/${id}/status?status_value=${status}`),
};

// Inventory Service APIs
export const inventoryAPI = {
  getAll: () => axios.get(`${INVENTORY_SERVICE_URL}/inventory`),
  getByProductId: (productId) => axios.get(`${INVENTORY_SERVICE_URL}/inventory/${productId}`),
  create: (data) => axios.post(`${INVENTORY_SERVICE_URL}/inventory`, data),
  update: (productId, data) => axios.put(`${INVENTORY_SERVICE_URL}/inventory/${productId}`, data),
};

// Notification Service APIs
export const notificationAPI = {
  getAll: (limit = 50) => axios.get(`${NOTIFICATION_SERVICE_URL}/notifications?limit=${limit}`),
  getByUserId: (userId, limit = 20) => axios.get(`${NOTIFICATION_SERVICE_URL}/notifications/user/${userId}?limit=${limit}`),
  sendTest: (message, userId = null) => axios.post(`${NOTIFICATION_SERVICE_URL}/notifications/test?message=${message}${userId ? `&user_id=${userId}` : ''}`),
};
