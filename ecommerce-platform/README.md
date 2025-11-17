# ShopAI - Production-Ready E-commerce Platform

A modern, full-stack e-commerce platform with AI/ML features, voice search, and comprehensive security. Built with React, Node.js, PostgreSQL, MongoDB, and Redis.

## Features

### Core Features
- **Multi-Category Product Catalog**: Toys, Electronics, Clothes, and more
- **Advanced Search**: Text search with autocomplete and filtering
- **Voice Search**: AI-powered voice search using Web Speech API
- **User Management**: Customer, Seller, and Admin roles
- **Shopping Cart**: Persistent cart with local storage
- **Wishlist**: Save products for later
- **Order Management**: Complete order lifecycle tracking
- **Payment Integration**: Stripe and PayPal support

### AI/ML Features
- **Personalized Recommendations**: Collaborative filtering based on purchase history
- **Smart Search**: AI-powered product search using OpenAI
- **Voice Search**: Natural language processing for voice queries
- **Similar Products**: Intelligent product recommendations
- **Sentiment Analysis**: Review sentiment tracking (placeholder for ML model)

### Security Features
- **JWT Authentication**: Secure token-based authentication
- **Password Hashing**: bcrypt with configurable rounds
- **Rate Limiting**: Protection against brute force attacks
- **CORS**: Configured cross-origin resource sharing
- **Input Validation**: Express-validator and Joi schemas
- **SQL Injection Prevention**: Sequelize ORM with parameterized queries
- **XSS Protection**: Input sanitization and helmet middleware
- **MongoDB Injection Prevention**: Express-mongo-sanitize

### Performance Features
- **Redis Caching**: Fast product and user data caching
- **Database Optimization**: Proper indexing and query optimization
- **Compression**: Gzip compression for API responses
- **Image Optimization**: Sharp for image processing
- **CDN Ready**: Static asset optimization

## Tech Stack

### Frontend
- **React 18** with TypeScript
- **Redux Toolkit** for state management
- **Material-UI (MUI)** for UI components
- **Vite** for fast development and building
- **React Router** for navigation
- **Formik & Yup** for form handling and validation
- **Axios** for API requests
- **Web Speech API** for voice search

### Backend
- **Node.js** with Express and TypeScript
- **PostgreSQL** (Sequelize ORM) for transactional data
- **MongoDB** (Mongoose ODM) for product catalog
- **Redis** (ioredis) for caching and sessions
- **JWT** for authentication
- **Winston** for logging
- **TensorFlow.js** for ML models
- **OpenAI API** for AI features

### Payment Gateways
- **Stripe** for card payments
- **PayPal** for PayPal payments

### DevOps
- **Docker** & **Docker Compose** for containerization
- **Nginx** for frontend serving
- **PM2** ready for production deployment

## Project Structure

```
ecommerce-platform/
├── backend/
│   ├── src/
│   │   ├── config/          # Database configurations
│   │   ├── controllers/     # Route controllers
│   │   ├── middleware/      # Express middleware
│   │   ├── models/          # Database models
│   │   ├── routes/          # API routes
│   │   ├── services/        # Business logic & AI services
│   │   ├── utils/           # Utility functions
│   │   └── server.ts        # Entry point
│   ├── Dockerfile
│   └── package.json
├── frontend/
│   ├── src/
│   │   ├── components/      # React components
│   │   ├── pages/           # Page components
│   │   ├── store/           # Redux store & slices
│   │   ├── services/        # API services
│   │   ├── theme/           # MUI theme
│   │   └── main.tsx         # Entry point
│   ├── Dockerfile
│   ├── nginx.conf
│   └── package.json
├── docker-compose.yml
└── README.md
```

## Getting Started

### Prerequisites
- **Node.js** 20+ and npm
- **Docker** and Docker Compose (for containerized deployment)
- **PostgreSQL** 16+ (if running locally)
- **MongoDB** 7+ (if running locally)
- **Redis** 7+ (if running locally)

### Environment Variables

Create `.env` files based on `.env.example`:

**Backend (.env)**:
```env
NODE_ENV=development
PORT=5000
FRONTEND_URL=http://localhost:3000

# Databases
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_USER=ecommerce_user
POSTGRES_PASSWORD=your_password
POSTGRES_DB=ecommerce_db

MONGODB_URI=mongodb://localhost:27017/ecommerce_products

REDIS_HOST=localhost
REDIS_PORT=6379

# JWT
JWT_SECRET=your_jwt_secret_key
JWT_EXPIRE=7d

# Payment
STRIPE_SECRET_KEY=sk_test_your_key
STRIPE_PUBLISHABLE_KEY=pk_test_your_key
PAYPAL_CLIENT_ID=your_client_id
PAYPAL_CLIENT_SECRET=your_client_secret

# AI (Optional)
OPENAI_API_KEY=your_openai_key
```

**Frontend (.env.local)**:
```env
VITE_API_URL=http://localhost:5000/api
```

### Installation & Development

#### Option 1: Docker Compose (Recommended)

1. Clone the repository
2. Copy `.env.example` to `.env` and configure
3. Run with Docker Compose:

```bash
# Build and start all services
docker-compose up -d

# View logs
docker-compose logs -f

# Stop services
docker-compose down
```

Services will be available at:
- Frontend: http://localhost:3000
- Backend: http://localhost:5000
- PostgreSQL: localhost:5432
- MongoDB: localhost:27017
- Redis: localhost:6379

#### Option 2: Local Development

1. Install dependencies:
```bash
npm run install:all
```

2. Start databases (PostgreSQL, MongoDB, Redis)

3. Start development servers:
```bash
# Start both frontend and backend
npm run dev

# Or separately
npm run dev:backend
npm run dev:frontend
```

### Production Deployment

#### Using Docker Compose

```bash
# Build production images
docker-compose build

# Start in production mode
docker-compose up -d

# Scale services
docker-compose up -d --scale backend=3
```

#### Manual Deployment

**Backend**:
```bash
cd backend
npm install
npm run build
NODE_ENV=production node dist/server.js
```

**Frontend**:
```bash
cd frontend
npm install
npm run build
# Serve dist/ with nginx or any static file server
```

## API Documentation

### Authentication
- `POST /api/auth/register` - Register new user
- `POST /api/auth/login` - Login
- `POST /api/auth/logout` - Logout
- `GET /api/auth/me` - Get current user
- `PUT /api/auth/update-profile` - Update profile
- `PUT /api/auth/update-password` - Update password
- `POST /api/auth/refresh-token` - Refresh JWT token

### Products
- `GET /api/products` - Get all products (with pagination, filters)
- `GET /api/products/featured` - Get featured products
- `GET /api/products/:id` - Get product by ID
- `POST /api/products` - Create product (seller/admin)
- `PUT /api/products/:id` - Update product (seller/admin)
- `DELETE /api/products/:id` - Delete product (seller/admin)

### AI Features
- `GET /api/ai/recommendations` - Get personalized recommendations
- `POST /api/ai/voice-search` - Process voice search query
- `GET /api/ai/search?query=` - Smart search
- `GET /api/ai/similar/:productId` - Get similar products

### Payments
- `POST /api/payments/create-intent` - Create payment intent
- `POST /api/payments/confirm` - Confirm payment
- `POST /api/payments/refund` - Create refund (admin)

### Orders
- `GET /api/orders` - Get user orders
- `GET /api/orders/:id` - Get order by ID
- `POST /api/orders` - Create order

## User Roles

1. **Customer**: Browse products, make purchases, manage cart/wishlist
2. **Seller**: All customer features + create/manage products
3. **Admin**: All features + user management, analytics

## Voice Search

The platform includes an advanced voice search feature:
- Click the microphone icon in the header
- Speak your search query (e.g., "Show me electronics under $500")
- AI processes the query and returns relevant products
- Supports natural language understanding

## Security Best Practices

1. **Environment Variables**: Never commit `.env` files
2. **JWT Secret**: Use strong, random secrets in production
3. **Database Passwords**: Use strong passwords
4. **HTTPS**: Always use HTTPS in production
5. **Rate Limiting**: Configured to prevent abuse
6. **Input Validation**: All inputs are validated
7. **SQL Injection**: Protected by ORM
8. **XSS**: Protected by sanitization middleware

## Performance Optimization

1. **Caching**: Redis caching for frequently accessed data
2. **Database Indexing**: Proper indexes on frequently queried fields
3. **Pagination**: All list endpoints support pagination
4. **Compression**: Gzip compression enabled
5. **CDN**: Static assets ready for CDN deployment

## Monitoring & Logging

- **Winston Logger**: Structured logging with rotation
- **Error Handling**: Centralized error handling
- **Health Checks**: `/health` endpoint for monitoring
- **Docker Health Checks**: Configured for all services

## Future Enhancements

- [ ] Admin dashboard for analytics
- [ ] Real-time notifications (WebSocket)
- [ ] Advanced ML recommendation models
- [ ] Image search capability
- [ ] Multi-language support
- [ ] Progressive Web App (PWA)
- [ ] Mobile apps (React Native)
- [ ] Advanced analytics and reporting
- [ ] Seller dashboard
- [ ] Social login (Google, Facebook)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

MIT License - feel free to use this project for learning and commercial purposes.

## Support

For issues and questions:
- Open an issue on GitHub
- Check existing documentation
- Review API endpoints in the code

## Acknowledgments

- Material-UI for the component library
- TensorFlow.js for ML capabilities
- Stripe & PayPal for payment processing
- OpenAI for AI features
- All open-source contributors

---

Built with ❤️ using modern web technologies
