import { useEffect } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { Link } from 'react-router-dom';
import {
  Container,
  Grid,
  Card,
  CardMedia,
  CardContent,
  CardActions,
  Typography,
  Button,
  Box,
  CircularProgress,
} from '@mui/material';
import { fetchFeaturedProducts, fetchRecommendations } from '../store/slices/productSlice';
import { RootState, AppDispatch } from '../store/store';

const Home = () => {
  const dispatch = useDispatch<AppDispatch>();
  const { featuredProducts, recommendations, loading } = useSelector(
    (state: RootState) => state.product
  );
  const { isAuthenticated } = useSelector((state: RootState) => state.auth);

  useEffect(() => {
    dispatch(fetchFeaturedProducts());
    if (isAuthenticated) {
      dispatch(fetchRecommendations());
    }
  }, [dispatch, isAuthenticated]);

  if (loading) {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '60vh' }}>
        <CircularProgress />
      </Box>
    );
  }

  return (
    <Container maxWidth="lg" sx={{ py: 4 }}>
      {/* Hero Section */}
      <Box
        sx={{
          bgcolor: 'primary.main',
          color: 'white',
          p: 6,
          borderRadius: 2,
          mb: 6,
          textAlign: 'center',
        }}
      >
        <Typography variant="h2" gutterBottom>
          Welcome to ShopAI
        </Typography>
        <Typography variant="h5" gutterBottom>
          Smart Shopping with AI-Powered Recommendations
        </Typography>
        <Button
          component={Link}
          to="/products"
          variant="contained"
          color="secondary"
          size="large"
          sx={{ mt: 2 }}
        >
          Shop Now
        </Button>
      </Box>

      {/* Featured Products */}
      <Typography variant="h4" gutterBottom sx={{ mb: 3 }}>
        Featured Products
      </Typography>
      <Grid container spacing={3} sx={{ mb: 6 }}>
        {featuredProducts.map((product) => (
          <Grid item xs={12} sm={6} md={4} lg={3} key={product._id}>
            <Card sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
              <CardMedia
                component="img"
                height="200"
                image={product.thumbnail || '/placeholder.jpg'}
                alt={product.name}
              />
              <CardContent sx={{ flexGrow: 1 }}>
                <Typography gutterBottom variant="h6" component="div" noWrap>
                  {product.name}
                </Typography>
                <Typography variant="body2" color="text.secondary" sx={{ mb: 1 }}>
                  {product.category}
                </Typography>
                <Typography variant="h6" color="primary">
                  ${product.price.toFixed(2)}
                </Typography>
              </CardContent>
              <CardActions>
                <Button
                  component={Link}
                  to={`/products/${product._id}`}
                  size="small"
                  fullWidth
                  variant="contained"
                >
                  View Details
                </Button>
              </CardActions>
            </Card>
          </Grid>
        ))}
      </Grid>

      {/* Recommendations for logged-in users */}
      {isAuthenticated && recommendations.length > 0 && (
        <>
          <Typography variant="h4" gutterBottom sx={{ mb: 3 }}>
            Recommended For You
          </Typography>
          <Grid container spacing={3}>
            {recommendations.map((product) => (
              <Grid item xs={12} sm={6} md={4} lg={3} key={product._id}>
                <Card sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
                  <CardMedia
                    component="img"
                    height="200"
                    image={product.thumbnail || '/placeholder.jpg'}
                    alt={product.name}
                  />
                  <CardContent sx={{ flexGrow: 1 }}>
                    <Typography gutterBottom variant="h6" component="div" noWrap>
                      {product.name}
                    </Typography>
                    <Typography variant="body2" color="text.secondary" sx={{ mb: 1 }}>
                      {product.category}
                    </Typography>
                    <Typography variant="h6" color="primary">
                      ${product.price.toFixed(2)}
                    </Typography>
                  </CardContent>
                  <CardActions>
                    <Button
                      component={Link}
                      to={`/products/${product._id}`}
                      size="small"
                      fullWidth
                      variant="contained"
                    >
                      View Details
                    </Button>
                  </CardActions>
                </Card>
              </Grid>
            ))}
          </Grid>
        </>
      )}
    </Container>
  );
};

export default Home;
