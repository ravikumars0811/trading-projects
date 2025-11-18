"use client"

import { MessageCircle } from "lucide-react"

export default function WhatsAppButton() {
  const whatsappNumber = process.env.NEXT_PUBLIC_WHATSAPP_NUMBER
  const message = encodeURIComponent("Hi! I'm interested in your construction services.")

  return (
    <a
      href={`https://wa.me/${whatsappNumber?.replace(/[^0-9]/g, '')}?text=${message}`}
      target="_blank"
      rel="noopener noreferrer"
      className="fixed bottom-6 right-6 bg-green-500 hover:bg-green-600 text-white rounded-full p-4 shadow-lg transition-all hover:scale-110 z-50 flex items-center gap-2"
      aria-label="Chat on WhatsApp"
    >
      <MessageCircle size={24} />
      <span className="hidden sm:inline font-medium">Chat with us</span>
    </a>
  )
}
