import axios from 'axios'

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:8000/api'

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
  timeout: 30000,
})

// Request interceptor
api.interceptors.request.use(
  (config) => {
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// Response interceptor
api.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error.response) {
      console.error('API Error:', error.response.data)
    } else if (error.request) {
      console.error('Network Error:', error.request)
    } else {
      console.error('Error:', error.message)
    }
    return Promise.reject(error)
  }
)

// API functions

export const healthCheck = async () => {
  const response = await api.get('/health')
  return response.data
}

export const priceOption = async (data) => {
  const response = await api.post('/pricing/option', data)
  return response.data
}

export const calculateImpliedVolatility = async (data) => {
  const response = await api.post('/pricing/implied-volatility', data)
  return response.data
}

export const priceBond = async (data) => {
  const response = await api.post('/pricing/bond', data)
  return response.data
}

export const priceSwap = async (data) => {
  const response = await api.post('/pricing/swap', data)
  return response.data
}

export default api
