import React from 'react'
import {
  Box,
  Card,
  CardContent,
  CardActions,
  Grid,
  Typography,
  Button,
  Paper,
} from '@mui/material'
import { useNavigate } from 'react-router-dom'
import ShowChartIcon from '@mui/icons-material/ShowChart'
import AccountBalanceIcon from '@mui/icons-material/AccountBalance'
import SwapHorizIcon from '@mui/icons-material/SwapHoriz'
import SpeedIcon from '@mui/icons-material/Speed'
import SecurityIcon from '@mui/icons-material/Security'
import CloudDoneIcon from '@mui/icons-material/CloudDone'

const HomePage = () => {
  const navigate = useNavigate()

  const features = [
    {
      title: 'Options Pricing',
      description: 'Price European and American options using Black-Scholes, Binomial Trees, and Monte Carlo methods',
      icon: <ShowChartIcon sx={{ fontSize: 60, color: 'primary.main' }} />,
      path: '/options',
    },
    {
      title: 'Fixed Income',
      description: 'Value bonds and calculate duration, convexity, and yield to maturity',
      icon: <AccountBalanceIcon sx={{ fontSize: 60, color: 'primary.main' }} />,
      path: '/bonds',
    },
    {
      title: 'Interest Rate Swaps',
      description: 'Price swaps and calculate fair swap rates with advanced yield curve modeling',
      icon: <SwapHorizIcon sx={{ fontSize: 60, color: 'primary.main' }} />,
      path: '/swaps',
    },
  ]

  const highlights = [
    {
      title: 'High Performance',
      description: 'C++ core engine for ultra-fast pricing calculations',
      icon: <SpeedIcon sx={{ fontSize: 40, color: 'secondary.main' }} />,
    },
    {
      title: 'Production Ready',
      description: 'Enterprise-grade architecture with comprehensive error handling',
      icon: <SecurityIcon sx={{ fontSize: 40, color: 'secondary.main' }} />,
    },
    {
      title: 'Cloud Deployable',
      description: 'Docker containers ready for AWS, Azure, or GCP deployment',
      icon: <CloudDoneIcon sx={{ fontSize: 40, color: 'secondary.main' }} />,
    },
  ]

  return (
    <Box>
      <Paper
        elevation={0}
        sx={{
          p: 4,
          mb: 4,
          background: 'linear-gradient(135deg, #1976d2 0%, #115293 100%)',
          color: 'white',
          borderRadius: 2,
        }}
      >
        <Typography variant="h2" gutterBottom>
          Pricing Engine
        </Typography>
        <Typography variant="h5" sx={{ opacity: 0.9 }}>
          Production-Ready Financial Instruments Valuation System
        </Typography>
        <Typography variant="body1" sx={{ mt: 2, opacity: 0.8 }}>
          Enterprise-grade pricing engine built with C++ for performance and Python for flexibility.
          Deployed as a full-stack solution with FastAPI backend and React frontend.
        </Typography>
      </Paper>

      <Typography variant="h4" gutterBottom sx={{ mb: 3 }}>
        Pricing Capabilities
      </Typography>

      <Grid container spacing={3} sx={{ mb: 6 }}>
        {features.map((feature, index) => (
          <Grid item xs={12} md={4} key={index}>
            <Card
              sx={{
                height: '100%',
                display: 'flex',
                flexDirection: 'column',
                transition: 'transform 0.2s',
                '&:hover': {
                  transform: 'translateY(-4px)',
                  boxShadow: 4,
                },
              }}
            >
              <CardContent sx={{ flexGrow: 1, textAlign: 'center' }}>
                <Box sx={{ mb: 2 }}>{feature.icon}</Box>
                <Typography variant="h5" component="h2" gutterBottom>
                  {feature.title}
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  {feature.description}
                </Typography>
              </CardContent>
              <CardActions sx={{ justifyContent: 'center', pb: 2 }}>
                <Button
                  variant="contained"
                  onClick={() => navigate(feature.path)}
                  size="large"
                >
                  Start Pricing
                </Button>
              </CardActions>
            </Card>
          </Grid>
        ))}
      </Grid>

      <Typography variant="h4" gutterBottom sx={{ mb: 3 }}>
        Key Features
      </Typography>

      <Grid container spacing={3}>
        {highlights.map((highlight, index) => (
          <Grid item xs={12} md={4} key={index}>
            <Card sx={{ height: '100%' }}>
              <CardContent sx={{ textAlign: 'center' }}>
                <Box sx={{ mb: 2 }}>{highlight.icon}</Box>
                <Typography variant="h6" gutterBottom>
                  {highlight.title}
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  {highlight.description}
                </Typography>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      <Box sx={{ mt: 6, p: 3, bgcolor: 'background.paper', borderRadius: 2 }}>
        <Typography variant="h5" gutterBottom>
          Technical Stack
        </Typography>
        <Grid container spacing={2} sx={{ mt: 1 }}>
          <Grid item xs={12} sm={6} md={3}>
            <Typography variant="subtitle1" color="primary" gutterBottom>
              Core Engine
            </Typography>
            <Typography variant="body2">C++17</Typography>
            <Typography variant="body2">pybind11</Typography>
          </Grid>
          <Grid item xs={12} sm={6} md={3}>
            <Typography variant="subtitle1" color="primary" gutterBottom>
              Backend
            </Typography>
            <Typography variant="body2">Python 3.11</Typography>
            <Typography variant="body2">FastAPI</Typography>
          </Grid>
          <Grid item xs={12} sm={6} md={3}>
            <Typography variant="subtitle1" color="primary" gutterBottom>
              Frontend
            </Typography>
            <Typography variant="body2">React 18</Typography>
            <Typography variant="body2">Material-UI</Typography>
          </Grid>
          <Grid item xs={12} sm={6} md={3}>
            <Typography variant="subtitle1" color="primary" gutterBottom>
              Deployment
            </Typography>
            <Typography variant="body2">Docker</Typography>
            <Typography variant="body2">Cloud Ready</Typography>
          </Grid>
        </Grid>
      </Box>
    </Box>
  )
}

export default HomePage
