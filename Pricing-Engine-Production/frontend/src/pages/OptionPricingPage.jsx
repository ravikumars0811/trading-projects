import React, { useState } from 'react'
import {
  Box,
  Button,
  Card,
  CardContent,
  FormControl,
  Grid,
  InputLabel,
  MenuItem,
  Select,
  TextField,
  Typography,
  Alert,
  CircularProgress,
  Divider,
  Paper,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableRow,
} from '@mui/material'
import { priceOption, calculateImpliedVolatility } from '../services/api'

const OptionPricingPage = () => {
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const [result, setResult] = useState(null)
  const [mode, setMode] = useState('price') // 'price' or 'iv'

  const [formData, setFormData] = useState({
    spot_price: '100',
    strike_price: '100',
    risk_free_rate: '0.05',
    volatility: '0.2',
    time_to_maturity: '1',
    dividend_yield: '0',
    option_type: 'call',
    option_style: 'european',
    pricing_model: 'black_scholes',
    num_steps: '100',
    num_simulations: '100000',
    market_price: '', // For IV calculation
  })

  const handleChange = (e) => {
    const { name, value } = e.target
    setFormData((prev) => ({
      ...prev,
      [name]: value,
    }))

    // Auto-adjust pricing model based on option style
    if (name === 'option_style' && value === 'american') {
      setFormData((prev) => ({
        ...prev,
        pricing_model: 'binomial',
      }))
    }
  }

  const handleSubmit = async (e) => {
    e.preventDefault()
    setLoading(true)
    setError(null)
    setResult(null)

    try {
      if (mode === 'price') {
        const data = {
          spot_price: parseFloat(formData.spot_price),
          strike_price: parseFloat(formData.strike_price),
          risk_free_rate: parseFloat(formData.risk_free_rate),
          volatility: parseFloat(formData.volatility),
          time_to_maturity: parseFloat(formData.time_to_maturity),
          dividend_yield: parseFloat(formData.dividend_yield),
          option_type: formData.option_type,
          option_style: formData.option_style,
          pricing_model: formData.pricing_model,
          num_steps: parseInt(formData.num_steps),
          num_simulations: parseInt(formData.num_simulations),
        }

        const response = await priceOption(data)
        setResult(response)
      } else {
        // Implied volatility
        const data = {
          spot_price: parseFloat(formData.spot_price),
          strike_price: parseFloat(formData.strike_price),
          risk_free_rate: parseFloat(formData.risk_free_rate),
          time_to_maturity: parseFloat(formData.time_to_maturity),
          dividend_yield: parseFloat(formData.dividend_yield),
          option_type: formData.option_type,
          market_price: parseFloat(formData.market_price),
        }

        const response = await calculateImpliedVolatility(data)
        setResult(response)
      }
    } catch (err) {
      setError(err.response?.data?.detail || err.message || 'An error occurred')
    } finally {
      setLoading(false)
    }
  }

  const resetForm = () => {
    setFormData({
      spot_price: '100',
      strike_price: '100',
      risk_free_rate: '0.05',
      volatility: '0.2',
      time_to_maturity: '1',
      dividend_yield: '0',
      option_type: 'call',
      option_style: 'european',
      pricing_model: 'black_scholes',
      num_steps: '100',
      num_simulations: '100000',
      market_price: '',
    })
    setResult(null)
    setError(null)
  }

  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        Options Pricing
      </Typography>
      <Typography variant="body1" color="text.secondary" paragraph>
        Price options using industry-standard models: Black-Scholes, Binomial Tree, and Monte Carlo
      </Typography>

      <Grid container spacing={3}>
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <FormControl fullWidth sx={{ mb: 2 }}>
                <InputLabel>Mode</InputLabel>
                <Select value={mode} onChange={(e) => setMode(e.target.value)} label="Mode">
                  <MenuItem value="price">Price Option</MenuItem>
                  <MenuItem value="iv">Calculate Implied Volatility</MenuItem>
                </Select>
              </FormControl>

              <form onSubmit={handleSubmit}>
                <Grid container spacing={2}>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Spot Price"
                      name="spot_price"
                      type="number"
                      value={formData.spot_price}
                      onChange={handleChange}
                      inputProps={{ step: '0.01', min: '0.01' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Strike Price"
                      name="strike_price"
                      type="number"
                      value={formData.strike_price}
                      onChange={handleChange}
                      inputProps={{ step: '0.01', min: '0.01' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Risk-Free Rate"
                      name="risk_free_rate"
                      type="number"
                      value={formData.risk_free_rate}
                      onChange={handleChange}
                      inputProps={{ step: '0.001', min: '0' }}
                      required
                    />
                  </Grid>
                  {mode === 'price' ? (
                    <Grid item xs={12} sm={6}>
                      <TextField
                        fullWidth
                        label="Volatility"
                        name="volatility"
                        type="number"
                        value={formData.volatility}
                        onChange={handleChange}
                        inputProps={{ step: '0.01', min: '0', max: '2' }}
                        required
                      />
                    </Grid>
                  ) : (
                    <Grid item xs={12} sm={6}>
                      <TextField
                        fullWidth
                        label="Market Price"
                        name="market_price"
                        type="number"
                        value={formData.market_price}
                        onChange={handleChange}
                        inputProps={{ step: '0.01', min: '0' }}
                        required
                      />
                    </Grid>
                  )}
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Time to Maturity (years)"
                      name="time_to_maturity"
                      type="number"
                      value={formData.time_to_maturity}
                      onChange={handleChange}
                      inputProps={{ step: '0.01', min: '0.01' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Dividend Yield"
                      name="dividend_yield"
                      type="number"
                      value={formData.dividend_yield}
                      onChange={handleChange}
                      inputProps={{ step: '0.01', min: '0' }}
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <FormControl fullWidth>
                      <InputLabel>Option Type</InputLabel>
                      <Select
                        name="option_type"
                        value={formData.option_type}
                        onChange={handleChange}
                        label="Option Type"
                      >
                        <MenuItem value="call">Call</MenuItem>
                        <MenuItem value="put">Put</MenuItem>
                      </Select>
                    </FormControl>
                  </Grid>

                  {mode === 'price' && (
                    <>
                      <Grid item xs={12} sm={6}>
                        <FormControl fullWidth>
                          <InputLabel>Option Style</InputLabel>
                          <Select
                            name="option_style"
                            value={formData.option_style}
                            onChange={handleChange}
                            label="Option Style"
                          >
                            <MenuItem value="european">European</MenuItem>
                            <MenuItem value="american">American</MenuItem>
                          </Select>
                        </FormControl>
                      </Grid>
                      <Grid item xs={12}>
                        <FormControl fullWidth>
                          <InputLabel>Pricing Model</InputLabel>
                          <Select
                            name="pricing_model"
                            value={formData.pricing_model}
                            onChange={handleChange}
                            label="Pricing Model"
                          >
                            <MenuItem
                              value="black_scholes"
                              disabled={formData.option_style === 'american'}
                            >
                              Black-Scholes (European only)
                            </MenuItem>
                            <MenuItem value="binomial">Binomial Tree</MenuItem>
                            <MenuItem value="monte_carlo">Monte Carlo</MenuItem>
                          </Select>
                        </FormControl>
                      </Grid>
                      {formData.pricing_model === 'binomial' && (
                        <Grid item xs={12}>
                          <TextField
                            fullWidth
                            label="Number of Steps"
                            name="num_steps"
                            type="number"
                            value={formData.num_steps}
                            onChange={handleChange}
                            inputProps={{ step: '1', min: '10', max: '1000' }}
                          />
                        </Grid>
                      )}
                      {formData.pricing_model === 'monte_carlo' && (
                        <Grid item xs={12}>
                          <TextField
                            fullWidth
                            label="Number of Simulations"
                            name="num_simulations"
                            type="number"
                            value={formData.num_simulations}
                            onChange={handleChange}
                            inputProps={{ step: '1000', min: '1000' }}
                          />
                        </Grid>
                      )}
                    </>
                  )}
                </Grid>

                <Box sx={{ mt: 3, display: 'flex', gap: 2 }}>
                  <Button
                    type="submit"
                    variant="contained"
                    size="large"
                    fullWidth
                    disabled={loading}
                  >
                    {loading ? <CircularProgress size={24} /> : mode === 'price' ? 'Calculate Price' : 'Calculate IV'}
                  </Button>
                  <Button variant="outlined" size="large" onClick={resetForm}>
                    Reset
                  </Button>
                </Box>
              </form>

              {error && (
                <Alert severity="error" sx={{ mt: 2 }}>
                  {error}
                </Alert>
              )}
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={6}>
          {result && (
            <Card className="pricing-result">
              <CardContent>
                <Typography variant="h5" gutterBottom>
                  Results
                </Typography>
                <Divider sx={{ my: 2 }} />

                {mode === 'price' ? (
                  <>
                    <Box sx={{ mb: 3 }}>
                      <Typography variant="h3" color="primary" gutterBottom>
                        ${result.price.toFixed(4)}
                      </Typography>
                      <Typography variant="caption" color="text.secondary">
                        Option Price
                      </Typography>
                    </Box>

                    <Typography variant="h6" gutterBottom sx={{ mt: 3 }}>
                      Greeks
                    </Typography>
                    <TableContainer component={Paper} variant="outlined">
                      <Table size="small">
                        <TableBody>
                          <TableRow>
                            <TableCell>
                              <strong>Delta (Δ)</strong>
                            </TableCell>
                            <TableCell align="right">
                              {result.greeks.delta.toFixed(6)}
                            </TableCell>
                          </TableRow>
                          <TableRow>
                            <TableCell>
                              <strong>Gamma (Γ)</strong>
                            </TableCell>
                            <TableCell align="right">
                              {result.greeks.gamma.toFixed(6)}
                            </TableCell>
                          </TableRow>
                          <TableRow>
                            <TableCell>
                              <strong>Theta (Θ)</strong>
                            </TableCell>
                            <TableCell align="right">
                              {result.greeks.theta.toFixed(6)}
                            </TableCell>
                          </TableRow>
                          <TableRow>
                            <TableCell>
                              <strong>Vega (ν)</strong>
                            </TableCell>
                            <TableCell align="right">
                              {result.greeks.vega.toFixed(6)}
                            </TableCell>
                          </TableRow>
                          <TableRow>
                            <TableCell>
                              <strong>Rho (ρ)</strong>
                            </TableCell>
                            <TableCell align="right">
                              {result.greeks.rho.toFixed(6)}
                            </TableCell>
                          </TableRow>
                        </TableBody>
                      </Table>
                    </TableContainer>

                    <Typography variant="caption" display="block" sx={{ mt: 2 }}>
                      Model: {result.model_used}
                    </Typography>
                  </>
                ) : (
                  <>
                    <Box sx={{ mb: 3 }}>
                      <Typography variant="h3" color="primary" gutterBottom>
                        {(result.implied_volatility * 100).toFixed(2)}%
                      </Typography>
                      <Typography variant="caption" color="text.secondary">
                        Implied Volatility
                      </Typography>
                    </Box>
                  </>
                )}

                <Typography variant="caption" display="block" color="text.secondary">
                  Calculated at: {new Date(result.timestamp).toLocaleString()}
                </Typography>
              </CardContent>
            </Card>
          )}

          {!result && (
            <Card sx={{ height: '100%', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
              <CardContent>
                <Typography variant="h6" color="text.secondary" align="center">
                  Fill in the form and click calculate to see results
                </Typography>
              </CardContent>
            </Card>
          )}
        </Grid>
      </Grid>
    </Box>
  )
}

export default OptionPricingPage
