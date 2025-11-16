import { useState, useEffect } from 'react';
import {
  IconButton,
  Dialog,
  DialogTitle,
  DialogContent,
  Box,
  Typography,
  CircularProgress,
} from '@mui/material';
import MicIcon from '@mui/icons-material/Mic';
import MicOffIcon from '@mui/icons-material/MicOff';
import { toast } from 'react-toastify';
import { aiAPI } from '../../services/api';
import { useNavigate } from 'react-router-dom';

interface VoiceSearchProps {
  onSearch?: (query: string) => void;
}

const VoiceSearch: React.FC<VoiceSearchProps> = ({ onSearch }) => {
  const [isListening, setIsListening] = useState(false);
  const [transcript, setTranscript] = useState('');
  const [isProcessing, setIsProcessing] = useState(false);
  const [open, setOpen] = useState(false);
  const navigate = useNavigate();

  useEffect(() => {
    // Check if browser supports speech recognition
    if (!('webkitSpeechRecognition' in window) && !('SpeechRecognition' in window)) {
      console.warn('Speech recognition not supported in this browser');
    }
  }, []);

  const startListening = () => {
    const SpeechRecognition =
      (window as any).SpeechRecognition || (window as any).webkitSpeechRecognition;

    if (!SpeechRecognition) {
      toast.error('Voice search is not supported in your browser');
      return;
    }

    const recognition = new SpeechRecognition();
    recognition.continuous = false;
    recognition.interimResults = true;
    recognition.lang = 'en-US';

    recognition.onstart = () => {
      setIsListening(true);
      setOpen(true);
      setTranscript('');
    };

    recognition.onresult = (event: any) => {
      const current = event.resultIndex;
      const transcriptText = event.results[current][0].transcript;
      setTranscript(transcriptText);

      // If final result, process it
      if (event.results[current].isFinal) {
        processVoiceQuery(transcriptText);
      }
    };

    recognition.onerror = (event: any) => {
      console.error('Speech recognition error:', event.error);
      setIsListening(false);
      toast.error('Voice recognition error. Please try again.');
    };

    recognition.onend = () => {
      setIsListening(false);
    };

    recognition.start();
  };

  const processVoiceQuery = async (query: string) => {
    setIsProcessing(true);

    try {
      const response = await aiAPI.voiceSearch(query);
      const { products } = response.data.data;

      if (onSearch) {
        onSearch(query);
      }

      // Navigate to products page with search results
      navigate('/products', { state: { voiceSearchResults: products, query } });

      toast.success(`Found ${products.length} products for "${query}"`);
      setOpen(false);
    } catch (error) {
      console.error('Voice search error:', error);
      toast.error('Failed to process voice search');
    } finally {
      setIsProcessing(false);
    }
  };

  return (
    <>
      <IconButton
        color="primary"
        onClick={startListening}
        disabled={isListening}
        title="Voice Search"
      >
        {isListening ? <MicIcon color="error" /> : <MicIcon />}
      </IconButton>

      <Dialog open={open} onClose={() => setOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Voice Search</DialogTitle>
        <DialogContent>
          <Box
            sx={{
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              gap: 3,
              py: 3,
            }}
          >
            {isListening && (
              <>
                <MicIcon sx={{ fontSize: 80, color: 'error.main', animation: 'pulse 1.5s infinite' }} />
                <Typography variant="h6" color="text.secondary">
                  Listening...
                </Typography>
              </>
            )}

            {isProcessing && (
              <>
                <CircularProgress />
                <Typography variant="h6" color="text.secondary">
                  Processing your request...
                </Typography>
              </>
            )}

            {transcript && (
              <Box sx={{ mt: 2, p: 2, bgcolor: 'grey.100', borderRadius: 2, width: '100%' }}>
                <Typography variant="body2" color="text.secondary">
                  You said:
                </Typography>
                <Typography variant="h6">{transcript}</Typography>
              </Box>
            )}

            <Typography variant="body2" color="text.secondary" textAlign="center">
              Try saying: "Show me electronics under $500" or "I need red running shoes"
            </Typography>
          </Box>
        </DialogContent>
      </Dialog>

      <style>{`
        @keyframes pulse {
          0%, 100% {
            opacity: 1;
          }
          50% {
            opacity: 0.5;
          }
        }
      `}</style>
    </>
  );
};

export default VoiceSearch;
