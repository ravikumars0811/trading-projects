import React from 'react';
import { BrowserRouter as Router, Routes, Route, Link } from 'react-router-dom';
import Dashboard from './pages/Dashboard';
import Users from './pages/Users';
import Products from './pages/Products';
import Orders from './pages/Orders';
import Notifications from './pages/Notifications';

function App() {
  return (
    <Router>
      <div className="App">
        <nav className="nav">
          <div className="container">
            <h1>Event-Driven Microservices Platform</h1>
            <ul className="nav-links">
              <li><Link to="/">Dashboard</Link></li>
              <li><Link to="/users">Users</Link></li>
              <li><Link to="/products">Products</Link></li>
              <li><Link to="/orders">Orders</Link></li>
              <li><Link to="/notifications">Notifications</Link></li>
            </ul>
          </div>
        </nav>

        <div className="container">
          <Routes>
            <Route path="/" element={<Dashboard />} />
            <Route path="/users" element={<Users />} />
            <Route path="/products" element={<Products />} />
            <Route path="/orders" element={<Orders />} />
            <Route path="/notifications" element={<Notifications />} />
          </Routes>
        </div>
      </div>
    </Router>
  );
}

export default App;
