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
import { priceSwap } from '../services/api'

const SwapPricingPage = () => {
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const [result, setResult] = useState(null)

  const [formData, setFormData] = useState({
    notional: '1000000',
    fixed_rate: '0.04',
    maturity: '5',
    payment_frequency: 'quarterly',
  })

  const [yieldCurve, setYieldCurve] = useState([
    { maturity: '0.25', rate: '0.03' },
    { maturity: '0.5', rate: '0.032' },
    { maturity: '1', rate: '0.035' },
    { maturity: '2', rate: '0.038' },
    { maturity: '3', rate: '0.04' },
    { maturity: '5', rate: '0.042' },
    { maturity: '7', rate: '0.044' },
    { maturity: '10', rate: '0.045' },
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
        notional: parseFloat(formData.notional),
        fixed_rate: parseFloat(formData.fixed_rate),
        maturity: parseFloat(formData.maturity),
        payment_frequency: formData.payment_frequency,
        yield_curve: yieldCurve.map((point) => ({
          maturity: parseFloat(point.maturity),
          rate: parseFloat(point.rate),
        })),
      }

      const response = await priceSwap(data)
      setResult(response)
    } catch (err) {
      setError(err.response?.data?.detail || err.message || 'An error occurred')
    } finally {
      setLoading(false)
    }
  }

  const resetForm = () => {
    setFormData({
      notional: '1000000',
      fixed_rate: '0.04',
      maturity: '5',
      payment_frequency: 'quarterly',
    })
    setYieldCurve([
      { maturity: '0.25', rate: '0.03' },
      { maturity: '0.5', rate: '0.032' },
      { maturity: '1', rate: '0.035' },
      { maturity: '2', rate: '0.038' },
      { maturity: '3', rate: '0.04' },
      { maturity: '5', rate: '0.042' },
      { maturity: '7', rate: '0.044' },
      { maturity: '10', rate: '0.045' },
    ])
    setResult(null)
    setError(null)
  }

  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        Interest Rate Swap Pricing
      </Typography>
      <Typography variant="body1" color="text.secondary" paragraph>
        Value interest rate swaps and calculate fair swap rates using yield curve analysis
      </Typography>

      <Grid container spacing={3}>
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Swap Parameters
              </Typography>
              <form onSubmit={handleSubmit}>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Notional Amount"
                      name="notional"
                      type="number"
                      value={formData.notional}
                      onChange={handleChange}
                      inputProps={{ step: '10000', min: '10000' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12} sm={6}>
                    <TextField
                      fullWidth
                      label="Fixed Rate"
                      name="fixed_rate"
                      type="number"
                      value={formData.fixed_rate}
                      onChange={handleChange}
                      inputProps={{ step: '0.001', min: '0', max: '1' }}
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
                      inputProps={{ step: '0.25', min: '0.25' }}
                      required
                    />
                  </Grid>
                  <Grid item xs={12}>
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

                <Box sx={{ maxHeight: '300px', overflowY: 'auto', pr: 1 }}>
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
                          inputProps={{ step: '0.25', min: '0' }}
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
                </Box>

                <Box sx={{ mt: 3, display: 'flex', gap: 2 }}>
                  <Button
                    type="submit"
                    variant="contained"
                    size="large"
                    fullWidth
                    disabled={loading}
                  >
                    {loading ? <CircularProgress size={24} /> : 'Calculate Swap Value'}
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
                  Swap Valuation Results
                </Typography>
                <Divider sx={{ my: 2 }} />

                <Box sx={{ mb: 3 }}>
                  <Typography variant="h3" color="primary" gutterBottom>
                    ${result.present_value.toLocaleString(undefined, {
                      minimumFractionDigits: 2,
                      maximumFractionDigits: 2,
                    })}
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    Present Value
                  </Typography>
                </Box>

                <TableContainer component={Paper} variant="outlined">
                  <Table size="small">
                    <TableBody>
                      <TableRow>
                        <TableCell>
                          <strong>Fair Swap Rate</strong>
                        </TableCell>
                        <TableCell align="right">
                          {(result.fair_swap_rate * 100).toFixed(4)}%
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
                          <strong>DV01</strong>
                        </TableCell>
                        <TableCell align="right">
                          ${result.dv01.toFixed(2)}
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Notional</strong>
                        </TableCell>
                        <TableCell align="right">
                          ${parseFloat(formData.notional).toLocaleString()}
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Fixed Rate</strong>
                        </TableCell>
                        <TableCell align="right">
                          {(parseFloat(formData.fixed_rate) * 100).toFixed(3)}%
                        </TableCell>
                      </TableRow>
                      <TableRow>
                        <TableCell>
                          <strong>Maturity</strong>
                        </TableCell>
                        <TableCell align="right">
                          {formData.maturity} years
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
                    • <strong>Fair Swap Rate:</strong> The fixed rate that makes the swap value zero at inception
                  </Typography>
                  <Typography variant="body2" color="text.secondary" paragraph>
                    • <strong>DV01:</strong> Dollar value change for a 1 basis point move in rates
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    • <strong>Present Value:</strong> Current market value of the swap (positive = receive fixed, negative = pay fixed)
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
                  Configure swap parameters and yield curve to calculate value
                </Typography>
              </CardContent>
            </Card>
          )}
        </Grid>
      </Grid>
    </Box>
  )
}

export default SwapPricingPage
