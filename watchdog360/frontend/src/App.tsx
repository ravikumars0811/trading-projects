import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom'
import { useStore } from './store/useStore'
import Login from './pages/Login'
import Dashboard from './pages/Dashboard'
import ServerDetail from './pages/ServerDetail'

function App() {
  const token = useStore((state) => state.token)

  return (
    <Router>
      <Routes>
        <Route
          path="/login"
          element={token ? <Navigate to="/dashboard" /> : <Login />}
        />
        <Route
          path="/dashboard"
          element={token ? <Dashboard /> : <Navigate to="/login" />}
        />
        <Route
          path="/server/:id"
          element={token ? <ServerDetail /> : <Navigate to="/login" />}
        />
        <Route path="/" element={<Navigate to="/dashboard" />} />
      </Routes>
    </Router>
  )
}

export default App
