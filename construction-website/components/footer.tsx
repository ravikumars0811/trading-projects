import Link from "next/link"
import { Facebook, Instagram, Youtube, Linkedin, Phone, Mail, MapPin } from "lucide-react"

export default function Footer() {
  return (
    <footer className="bg-gray-900 text-white">
      <div className="container mx-auto px-4 py-12">
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-8">
          {/* Company Info */}
          <div>
            <h3 className="text-xl font-bold mb-4">{process.env.NEXT_PUBLIC_COMPANY_NAME}</h3>
            <p className="text-gray-400 mb-4">
              Building dreams into reality with quality construction, transparency, and timely delivery.
            </p>
            <div className="flex gap-4">
              <a href="#" className="hover:text-primary transition-colors">
                <Facebook size={20} />
              </a>
              <a href="#" className="hover:text-primary transition-colors">
                <Instagram size={20} />
              </a>
              <a href="#" className="hover:text-primary transition-colors">
                <Youtube size={20} />
              </a>
              <a href="#" className="hover:text-primary transition-colors">
                <Linkedin size={20} />
              </a>
            </div>
          </div>

          {/* Quick Links */}
          <div>
            <h3 className="text-lg font-semibold mb-4">Quick Links</h3>
            <ul className="space-y-2">
              <li><Link href="/about" className="text-gray-400 hover:text-white transition-colors">About Us</Link></li>
              <li><Link href="/services" className="text-gray-400 hover:text-white transition-colors">Services</Link></li>
              <li><Link href="/projects" className="text-gray-400 hover:text-white transition-colors">Projects</Link></li>
              <li><Link href="/gallery" className="text-gray-400 hover:text-white transition-colors">Gallery</Link></li>
              <li><Link href="/blog" className="text-gray-400 hover:text-white transition-colors">Blog</Link></li>
              <li><Link href="/contact" className="text-gray-400 hover:text-white transition-colors">Contact</Link></li>
            </ul>
          </div>

          {/* Services */}
          <div>
            <h3 className="text-lg font-semibold mb-4">Our Services</h3>
            <ul className="space-y-2">
              <li><Link href="/services/house-construction" className="text-gray-400 hover:text-white transition-colors">House Construction</Link></li>
              <li><Link href="/services/villas" className="text-gray-400 hover:text-white transition-colors">Villa Construction</Link></li>
              <li><Link href="/services/rental-buildings" className="text-gray-400 hover:text-white transition-colors">Rental Buildings</Link></li>
              <li><Link href="/packages" className="text-gray-400 hover:text-white transition-colors">Construction Packages</Link></li>
              <li><Link href="/calculator" className="text-gray-400 hover:text-white transition-colors">Cost Calculator</Link></li>
            </ul>
          </div>

          {/* Contact Info */}
          <div>
            <h3 className="text-lg font-semibold mb-4">Contact Us</h3>
            <ul className="space-y-3">
              <li className="flex items-start gap-2">
                <MapPin size={20} className="mt-1 flex-shrink-0" />
                <span className="text-gray-400">{process.env.NEXT_PUBLIC_COMPANY_ADDRESS}</span>
              </li>
              <li className="flex items-center gap-2">
                <Phone size={20} className="flex-shrink-0" />
                <a href={`tel:${process.env.NEXT_PUBLIC_COMPANY_PHONE}`} className="text-gray-400 hover:text-white transition-colors">
                  {process.env.NEXT_PUBLIC_COMPANY_PHONE}
                </a>
              </li>
              <li className="flex items-center gap-2">
                <Mail size={20} className="flex-shrink-0" />
                <a href={`mailto:${process.env.NEXT_PUBLIC_COMPANY_EMAIL}`} className="text-gray-400 hover:text-white transition-colors">
                  {process.env.NEXT_PUBLIC_COMPANY_EMAIL}
                </a>
              </li>
            </ul>
            <div className="mt-4">
              <p className="text-sm text-gray-400">Working Hours:</p>
              <p className="text-sm text-gray-300">Mon - Sat: 9:00 AM - 6:00 PM</p>
            </div>
          </div>
        </div>

        {/* Bottom Bar */}
        <div className="border-t border-gray-800 mt-8 pt-8 text-center text-gray-400 text-sm">
          <p>&copy; {new Date().getFullYear()} {process.env.NEXT_PUBLIC_COMPANY_NAME}. All rights reserved.</p>
          <div className="mt-2">
            <Link href="/privacy" className="hover:text-white transition-colors">Privacy Policy</Link>
            <span className="mx-2">|</span>
            <Link href="/terms" className="hover:text-white transition-colors">Terms of Service</Link>
          </div>
        </div>
      </div>
    </footer>
  )
}
