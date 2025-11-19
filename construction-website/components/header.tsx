"use client"

import Link from "next/link"
import { useState } from "react"
import { Menu, X, Phone, Mail } from "lucide-react"
import { Button } from "@/components/ui/button"

const navigation = [
  { name: "Home", href: "/" },
  { name: "About", href: "/about" },
  {
    name: "Services",
    href: "/services",
    submenu: [
      { name: "House Construction", href: "/services/house-construction" },
      { name: "Villas", href: "/services/villas" },
      { name: "Rental Buildings", href: "/services/rental-buildings" },
    ]
  },
  { name: "Projects", href: "/projects" },
  { name: "Gallery", href: "/gallery" },
  { name: "Packages", href: "/packages" },
  { name: "Cost Calculator", href: "/calculator" },
  { name: "Blog", href: "/blog" },
  { name: "Contact", href: "/contact" },
]

export default function Header() {
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false)

  return (
    <header className="bg-white shadow-md sticky top-0 z-50">
      {/* Top Bar */}
      <div className="bg-primary text-white">
        <div className="container mx-auto px-4 py-2">
          <div className="flex justify-between items-center text-sm">
            <div className="flex gap-4">
              <a href={`tel:${process.env.NEXT_PUBLIC_COMPANY_PHONE}`} className="flex items-center gap-1 hover:underline">
                <Phone size={14} />
                {process.env.NEXT_PUBLIC_COMPANY_PHONE}
              </a>
              <a href={`mailto:${process.env.NEXT_PUBLIC_COMPANY_EMAIL}`} className="flex items-center gap-1 hover:underline">
                <Mail size={14} />
                {process.env.NEXT_PUBLIC_COMPANY_EMAIL}
              </a>
            </div>
            <div className="hidden md:flex gap-3">
              <Link href="/login" className="hover:underline">Login</Link>
              <span>|</span>
              <Link href="/admin/login" className="hover:underline">Admin</Link>
            </div>
          </div>
        </div>
      </div>

      {/* Main Navigation */}
      <nav className="container mx-auto px-4 py-4">
        <div className="flex justify-between items-center">
          <Link href="/" className="text-2xl font-bold text-primary">
            {process.env.NEXT_PUBLIC_COMPANY_NAME}
          </Link>

          {/* Desktop Navigation */}
          <div className="hidden lg:flex items-center gap-6">
            {navigation.map((item) => (
              <div key={item.name} className="relative group">
                <Link
                  href={item.href}
                  className="text-gray-700 hover:text-primary font-medium transition-colors"
                >
                  {item.name}
                </Link>
                {item.submenu && (
                  <div className="absolute hidden group-hover:block bg-white shadow-lg rounded-md py-2 mt-2 min-w-[200px]">
                    {item.submenu.map((subitem) => (
                      <Link
                        key={subitem.name}
                        href={subitem.href}
                        className="block px-4 py-2 text-sm text-gray-700 hover:bg-gray-100"
                      >
                        {subitem.name}
                      </Link>
                    ))}
                  </div>
                )}
              </div>
            ))}
            <Button asChild>
              <Link href="/contact">Get Quote</Link>
            </Button>
          </div>

          {/* Mobile menu button */}
          <button
            className="lg:hidden"
            onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
          >
            {mobileMenuOpen ? <X size={24} /> : <Menu size={24} />}
          </button>
        </div>

        {/* Mobile Navigation */}
        {mobileMenuOpen && (
          <div className="lg:hidden mt-4 pb-4">
            {navigation.map((item) => (
              <div key={item.name} className="py-2">
                <Link
                  href={item.href}
                  className="block text-gray-700 hover:text-primary font-medium"
                  onClick={() => setMobileMenuOpen(false)}
                >
                  {item.name}
                </Link>
                {item.submenu && (
                  <div className="pl-4 mt-2 space-y-2">
                    {item.submenu.map((subitem) => (
                      <Link
                        key={subitem.name}
                        href={subitem.href}
                        className="block text-sm text-gray-600 hover:text-primary"
                        onClick={() => setMobileMenuOpen(false)}
                      >
                        {subitem.name}
                      </Link>
                    ))}
                  </div>
                )}
              </div>
            ))}
            <Button asChild className="w-full mt-4">
              <Link href="/contact">Get Quote</Link>
            </Button>
          </div>
        )}
      </nav>
    </header>
  )
}
