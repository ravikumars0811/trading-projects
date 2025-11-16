import { Box, Container, Typography, Grid, Link } from '@mui/material';

const Footer = () => {
  return (
    <Box
      component="footer"
      sx={{
        py: 4,
        px: 2,
        mt: 'auto',
        backgroundColor: (theme) => theme.palette.grey[200],
      }}
    >
      <Container maxWidth="lg">
        <Grid container spacing={4}>
          <Grid item xs={12} sm={4}>
            <Typography variant="h6" gutterBottom>
              ShopAI
            </Typography>
            <Typography variant="body2" color="text.secondary">
              Your smart shopping companion with AI-powered recommendations and voice search.
            </Typography>
          </Grid>
          <Grid item xs={12} sm={4}>
            <Typography variant="h6" gutterBottom>
              Quick Links
            </Typography>
            <Link href="/products" color="inherit" display="block" underline="hover">
              Products
            </Link>
            <Link href="/about" color="inherit" display="block" underline="hover">
              About Us
            </Link>
            <Link href="/contact" color="inherit" display="block" underline="hover">
              Contact
            </Link>
          </Grid>
          <Grid item xs={12} sm={4}>
            <Typography variant="h6" gutterBottom>
              Support
            </Typography>
            <Link href="/faq" color="inherit" display="block" underline="hover">
              FAQ
            </Link>
            <Link href="/shipping" color="inherit" display="block" underline="hover">
              Shipping Info
            </Link>
            <Link href="/returns" color="inherit" display="block" underline="hover">
              Returns
            </Link>
          </Grid>
        </Grid>
        <Box sx={{ mt: 3, textAlign: 'center' }}>
          <Typography variant="body2" color="text.secondary">
            © {new Date().getFullYear()} ShopAI. All rights reserved.
          </Typography>
        </Box>
      </Container>
    </Box>
  );
};

export default Footer;
