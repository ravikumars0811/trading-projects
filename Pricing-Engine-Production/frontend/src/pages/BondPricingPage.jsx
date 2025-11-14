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
  IconButton,
} from '@mui/material'
import AddIcon from '@mui/icons-material/Add'
import DeleteIcon from '@mui/icons-material/Delete'
import { priceBond } from '../services/api'

const BondPricingPage = () => {
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const [result, setResult] = useState(null)

  const [formData, setFormData] = useState({
    face_value: '1000',
    maturity: '5',
    coupon_rate: '0.05',
    payment_frequency: 'semi_annual',
  })

  const [yieldCurve, setYieldCurve] = useState([
    { maturity: '0.5', rate: '0.03' },
    { maturity: '1', rate: '0.035' },
    { maturity: '2', rate: '0.04' },
    { maturity: '5', rate: '0.045' },
    { maturity: '10', rate: '0.05' },
  ])

  const handleChange = (e) => {
    const { name, value } = e.target
    setFormData((prev) => ({
      ...prev,
      [name]: value,
    }))
  }

  const handleYieldCurveChange = (index, field, value) => {
    const newCurve = [...yieldCurve]
    newCurve[index][field] = value
    setYieldCurve(newCurve)
  }

  const addYieldCurvePoint = () => {
    setYieldCurve([...yieldCurve, { maturity: '', rate: '' }])
  }

  const removeYieldCurvePoint = (index) => {
    if (yieldCurve.length > 1) {
      const newCurve = yieldCurve.filter((_, i) => i !== index)
      setYieldCurve(newCurve)
    }
  }

  const handleSubmit = async (e) => {
    e.preventDefault()
    setLoading(true)
    setError(null)
    setResult(null)

    try {
      const data = {
        face_value: parseFloat(formData.face_value),
        maturity: parseFloat(formData.maturity),
        coupon_rate: parseFloat(formData.coupon_rate),
        payment_frequency: formData.payment_frequency,
        yield_curve: yieldCurve.map((point) => ({
          maturity: parseFloat(point.maturity),
          rate: parseFloat(point.rate),
        })),
      }

      const response = await priceBond(data)
      setResult(response)
    } catch (err) {
      setError(err.response?.data?.detail || err.message || 'An error occurred')
    } finally {
      setLoading(false)
    }
  }

  const resetForm = () => {
    setFormData({
      face_value: '1000',
      maturity: '5',
      coupon_rate: '0.05',
      payment_frequency: 'semi_annual',
    })
    setYieldCurve([
      { maturity: '0.5', rate: '0.03' },
      { maturity: '1', rate: '0.035' },
      { maturity: '2', rate: '0.04' },
      { maturity: '5', rate: '0.045' },
      { maturity: '10', rate: '0.05' },
    ])
    setResult(null)
    setError(null)
  }

  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        Bond Pricing
      </Typography>
      <Typography variant="body1" color="text.secondary" paragraph>
        Price zero-coupon and coupon-bearing bonds with duration and convexity analysis
      </Typography>

      <Grid container spacing={3}>
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Bond Parameters
              </Typography>
              <form onSubmit={handleSubmit}>
                <Grid container spacing={2}>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Face Value"
                      name="face_value"
                      type="number"
                      value={formData.face_value}
                      onChange={handleChange}
                      inputProps={{ step: '100', min: '100' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Maturity (years)"
                      name="maturity"
                      type="number"
                      value={formData.maturity}
                      onChange={handleChange}
                      inputProps={{ step: '0.5', min: '0.5' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Coupon Rate (0 for zero-coupon)"
                      name="coupon_rate"
                      type="number"
                      value={formData.coupon_rate}
                      onChange={handleChange}
                      inputProps={{ step: '0.01', min: '0', max: '1' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <FormControl fullWidth>
                      <InputLabel>Payment Frequency</InputLabel>
                      <Select
                        name="payment_frequency"
                        value={formData.payment_frequency}
                        onChange={handleChange}
                        label="Payment Frequency"
                      >
                        <MenuItem value="annual">Annual</MenuItem>
                        <MenuItem value="semi_annual">Semi-Annual</MenuItem>
                        <MenuItem value="quarterly">Quarterly</MenuItem>
                        <MenuItem value="monthly">Monthly</MenuItem>
                      </Select>
                    </FormControl>
                  </Grid>
                </Grid>

                <Divider sx={{ my: 3 }} />

                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
                  <Typography variant="h6">Yield Curve</Typography>
                  <Button
                    startIcon={<AddIcon />}
                    onClick={addYieldCurvePoint}
                    size="small"
                  >
                    Add Point
                  </Button>
                </Box>

                {yieldCurve.map((point, index) => (
                  <Grid container spacing={2} key={index} sx={{ mb: 1 }}>
                    <Grid item xs={5}>
                      <TextField
                        fullWidth
                        label="Maturity (years)"
                        type="number"
                        value={point.maturity}
                        onChange={(e) =>
                          handleYieldCurveChange(index, 'maturity', e.target.value)
                        }
                        inputProps={{ step: '0.5', min: '0' }}
                        required
                        size="small"
                      />
                    </Grid>
                    <Grid item xs={5}>
                      <TextField
                        fullWidth
                        label="Rate"
                        type="number"
                        value={point.rate}
                        onChange={(e) =>
                          handleYieldCurveChange(index, 'rate', e.target.value)
                        }
                        inputProps={{ step: '0.001', min: '0' }}
                        required
                        size="small"
                      />
                    </Grid>
                    <Grid item xs={2}>
                      <IconButton
                        onClick={() => removeYieldCurvePoint(index)}
                        disabled={yieldCurve.length === 1}
                        color="error"
                      >
                        <DeleteIcon />
                      </IconButton>
                    </Grid>
                  </Grid>
                ))}

                <Box sx={{ mt: 3, display: 'flex', gap: 2 }}>
                  <Button
                    type="submit"
                    variant="contained"
                    size="large"
                    fullWidth
                    disabled={loading}
                  >
                    {loading ? <CircularProgress size={24} /> : 'Calculate Bond Price'}
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
                  Bond Valuation Results
                </Typography>
                <Divider sx={{ my: 2 }} />

                <Box sx={{ mb: 3 }}>
                  <Typography variant="h3" color="primary" gutterBottom>
                    ${result.price.toFixed(2)}
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    Bond Price
                  </Typography>
                </Box>

                <TableContainer component={Paper} variant="outlined">
                  <Table size="small">
                    <TableBody>
                      <TableRow>
                        <TableCell>
                          <strong>Yield to Maturity</strong>
                        </TableCell>
                        <TableCell align="right">
                          {(result.yield_to_maturity * 100).toFixed(3)}%
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Duration</strong>
                        </TableCell>
                        <TableCell align="right">
                          {result.duration.toFixed(4)} years
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Convexity</strong>
                        </TableCell>
                        <TableCell align="right">
                          {result.convexity.toFixed(4)}
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Face Value</strong>
                        </TableCell>
                        <TableCell align="right">
                          ${parseFloat(formData.face_value).toFixed(2)}
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Coupon Rate</strong>
                        </TableCell>
                        <TableCell align="right">
                          {(parseFloat(formData.coupon_rate) * 100).toFixed(2)}%
                        </TableCell>
                      </TableRow>
                    </TableBody>
                  </Table>
                </TableContainer>

                <Box sx={{ mt: 3, p: 2, bgcolor: 'background.default', borderRadius: 1 }}>
                  <Typography variant="subtitle2" gutterBottom>
                    Key Insights:
                  </Typography>
                  <Typography variant="body2" color="text.secondary" paragraph>
                    • Duration measures the bond's price sensitivity to interest rate changes
                  </Typography>
                  <Typography variant="body2" color="text.secondary" paragraph>
                    • Convexity measures the curvature of the price-yield relationship
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    • YTM is the total return anticipated if the bond is held until maturity
                  </Typography>
                </Box>

                <Typography variant="caption" display="block" color="text.secondary" sx={{ mt: 2 }}>
                  Calculated at: {new Date(result.timestamp).toLocaleString()}
                </Typography>
              </CardContent>
            </Card>
          )}

          {!result && (
            <Card sx={{ height: '100%', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
              <CardContent>
                <Typography variant="h6" color="text.secondary" align="center">
                  Configure bond parameters and yield curve to calculate price
                </Typography>
              </CardContent>
            </Card>
          )}
        </Grid>
      </Grid>
    </Box>
  )
}

export default BondPricingPage
