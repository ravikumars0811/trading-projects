# BuildPro Construction Website

A comprehensive, production-ready construction company website built with Next.js 14, TypeScript, Tailwind CSS, and PostgreSQL.

## 🚀 Features

### ✅ Implemented Features

**Public-Facing Pages:**
- ✅ **Home Page** - Hero banner, services overview, packages, testimonials, stats, CTAs
- ✅ **About Us** - Company info, mission, vision, team, core values
- ✅ **Cost Calculator** - Interactive calculator with real-time estimates and breakdown
- ✅ **Contact Page** - Contact form, company info, map integration ready
- ✅ **Responsive Design** - Mobile-first, fully responsive across all devices
- ✅ **WhatsApp Integration** - Floating button with pre-filled messages

**Technical Implementation:**
- ✅ Next.js 14 with App Router and Server Components
- ✅ TypeScript for type safety
- ✅ Tailwind CSS with custom design system
- ✅ Prisma ORM with PostgreSQL database
- ✅ API routes for contact and enquiry forms
- ✅ Comprehensive database schema for full-featured website
- ✅ Reusable UI components (Button, Card, Input, etc.)
- ✅ SEO-optimized pages with proper meta tags

### 📋 Database Schema (Ready for Implementation)

The database is designed to support:
- User authentication (Admin & Client roles)
- Services management
- Construction packages (Basic, Standard, Premium, Luxury)
- Projects portfolio (Completed, Ongoing, Upcoming)
- Testimonials with video support
- Lead/enquiry management
- Blog/articles system
- Gallery with categorization
- Client portal for project tracking
- Support request system
- Document downloads
- Site settings configuration

### 🎯 Features Ready to Implement

The following features have backend support and need frontend implementation:

1. **Service Pages** - Detailed pages for Houses, Villas, Rental Buildings
2. **Projects Portfolio** - Showcase completed, ongoing, and upcoming projects
3. **Gallery** - Filtered photo and video gallery
4. **Packages Page** - Detailed package comparison with materials list
5. **Blog System** - Articles, tips, and guides
6. **Admin Dashboard** - Full CRUD operations for all content
7. **Customer Portal** - Login, project tracking, documents, payments
8. **Authentication** - NextAuth.js integration
9. **Email Notifications** - Nodemailer integration
10. **Analytics** - Google Analytics and Facebook Pixel
11. **Review System** - Customer reviews and ratings

## 🛠️ Tech Stack

- **Framework:** Next.js 14 (App Router)
- **Language:** TypeScript
- **Styling:** Tailwind CSS
- **Database:** PostgreSQL
- **ORM:** Prisma
- **Authentication:** NextAuth.js v5 (ready to configure)
- **Icons:** Lucide React
- **Deployment:** Vercel-ready

## 📦 Getting Started

### Prerequisites

- Node.js 18+
- PostgreSQL database
- npm or yarn

### Installation

1. **Clone and install**
   ```bash
   cd construction-website
   npm install
   ```

2. **Configure environment variables**

   Copy `.env.example` to `.env` and update:

   ```env
   # Database
   DATABASE_URL="postgresql://user:password@localhost:5432/construction_db"

   # Company Info (customize these)
   NEXT_PUBLIC_COMPANY_NAME="BuildPro Construction"
   NEXT_PUBLIC_COMPANY_EMAIL="info@buildpro.com"
   NEXT_PUBLIC_COMPANY_PHONE="+1 234 567 8900"
   NEXT_PUBLIC_COMPANY_ADDRESS="123 Construction Street, City"
   NEXT_PUBLIC_WHATSAPP_NUMBER="+1234567890"

   # Optional (for full features)
   NEXT_PUBLIC_GA_ID=""
   NEXT_PUBLIC_FB_PIXEL_ID=""
   NEXT_PUBLIC_GOOGLE_MAPS_API_KEY=""
   ```

3. **Set up database**
   ```bash
   npx prisma generate
   npx prisma db push
   ```

4. **Run development server**
   ```bash
   npm run dev
   ```

5. **Open browser**

   Visit [http://localhost:3000](http://localhost:3000)

## 📁 Project Structure

```
construction-website/
├── app/                    # Next.js App Router
│   ├── page.tsx           # Home page ✅
│   ├── about/             # About page ✅
│   ├── calculator/        # Cost calculator ✅
│   ├── contact/           # Contact page ✅
│   ├── services/          # Service pages (structure ready)
│   ├── api/               # API routes ✅
│   │   ├── contact/       # Contact form API ✅
│   │   └── enquiry/       # Enquiry API ✅
│   └── layout.tsx         # Root layout ✅
├── components/            # React components
│   ├── ui/               # UI components ✅
│   ├── header.tsx        # Navigation ✅
│   ├── footer.tsx        # Footer ✅
│   └── whatsapp-button.tsx ✅
├── lib/                  # Utilities
│   ├── prisma.ts        # Prisma client ✅
│   └── utils.ts         # Helper functions ✅
├── prisma/
│   └── schema.prisma    # Database schema ✅
├── .env.example         # Environment template ✅
└── README.md           # This file ✅
```

## 🎨 Customization

### Branding
Update `.env` file with your company information.

### Colors
Modify CSS variables in `app/globals.css`:
```css
:root {
  --primary: 221.2 83.2% 53.3%;  /* Blue */
  --secondary: 210 40% 96.1%;
  /* ... more colors */
}
```

### Calculator Pricing
Edit `lib/utils.ts` to adjust:
- Package base prices
- Location factors
- Timeline calculations

## 📊 Database Setup

### Local Development
```bash
# Install PostgreSQL locally, then:
npx prisma generate
npx prisma db push
npx prisma studio  # Open database GUI
```

### Production Database Options
- **Vercel Postgres** (easiest with Vercel)
- **Supabase** (generous free tier)
- **Railway** (simple setup)
- **AWS RDS** (enterprise-grade)

## 🚀 Deployment

### Deploy to Vercel

1. Push code to GitHub
2. Import project in Vercel
3. Add environment variables
4. Deploy

The app is optimized for Vercel with:
- Edge runtime support
- Automatic API routes
- Image optimization
- Built-in analytics

## 💡 Key Features Explained

### Cost Calculator
- Real-time estimation based on:
  - Site size (sq.ft)
  - Number of floors
  - Package type
  - Location factor
- Detailed breakdown of materials, labor, and other costs
- Timeline estimation
- Ready for PDF export (needs implementation)

### Database Schema
Comprehensive schema includes:
- **14 models** covering all website needs
- User authentication with roles
- Lead management with status tracking
- Project portfolio with progress tracking
- Blog with categories and tags
- Customer portal functionality
- Support ticket system

### API Routes
- `POST /api/contact` - Contact form submissions
- `POST /api/enquiry` - General enquiries
- `GET /api/enquiry` - Fetch enquiries (admin)

## 📝 Available Scripts

```bash
npm run dev          # Start development server
npm run build        # Build for production
npm start            # Start production server
npm run lint         # Run ESLint

# Database commands
npx prisma studio    # Database GUI
npx prisma generate  # Generate Prisma Client
npx prisma db push   # Push schema to database
```

## 🔐 Security

- Environment variables for sensitive data
- SQL injection prevention via Prisma
- Type safety with TypeScript
- Input validation on forms
- Ready for HTTPS in production

## 📈 Performance

- Server Components for faster initial load
- Code splitting by route
- Optimized images
- Minimal JavaScript bundle
- Edge-ready architecture

## 🎯 Next Steps

To complete the website:

1. **Implement remaining pages:**
   - Services detail pages
   - Projects portfolio
   - Gallery
   - Blog
   - Packages comparison

2. **Add admin dashboard:**
   - Use the existing database schema
   - Implement CRUD operations
   - Add authentication

3. **Enable customer portal:**
   - Client login
   - Project tracking
   - Document access

4. **Add integrations:**
   - Email notifications (Nodemailer ready)
   - Google Analytics
   - reCAPTCHA
   - Payment gateway

5. **Content:**
   - Add real images
   - Write actual content
   - Populate database with sample data

## 📞 Support

For issues or questions:
- Review code comments
- Check Prisma schema documentation
- Consult Next.js 14 documentation

## 📄 License

Private and proprietary.

---

**Built with modern web technologies for the construction industry.**

Ready for production deployment and further customization.
