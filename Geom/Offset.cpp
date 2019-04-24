////////////////////////////////////////////////////////////////////////////////////////////////
//                    2d geometry classes - implements 2d CCurve offset for use in dll
//
//                    g.j.hawkesford August 2003
//
// This program is released under the BSD license. See the file COPYING for details.
//
////////////////////////////////////////////////////////////////////////////////////////////////
#include "geometry.h"
#include "Curve.h"

static CCurve eliminateLoops(const CCurve& k , const CCurve& originalk, double offset, int& ret);
	static bool DoesIntersInterfere(const Point& pInt, const CCurve& k, double offset);

	enum SPAN_IDS {
		UNMARKED = 0xe0000000,
		ROLL_AROUND,
		INTERSECTION,
		FULL_CIRCLE_KURVE
	};


	int CCurve::OffsetMethod1(CCurve& kOffset, double off, int direction, int& ret)const
	{
		// offset CCurve with simple span elimination
		// direction 1 = left,  -1 = right

		// ret  = 0		- CCurve offset ok
		//		= 1		- CCurve has differential scale (not allowed)
		//		= 2		- offset failed
		//      = 3		- offset too large
		if(this == &kOffset) FAILURE(L"Illegal Call - 'this' must not be kOffset");
		double offset = (direction == GEOFF_LEFT)?off : -off;

		if (fabs(offset) < TOLERANCE || m_vertices.size() < 2) {
			kOffset = *this;
			ret = 0;
			return 1;
		}

		Span curSpan, curSpanOff;	// current & offset spans
		Span prevSpanOff;			// previous offset span

		// offset CCurve
		kOffset = CCurve();

		int RollDir = ( off < 0 ) ? direction : - direction;				// Roll arc direction

		std::list<Span> spans;
		GetSpans(spans);

		bool bClosed = IsClosed();
		unsigned int nspans = spans.size();
		if(bClosed) {
			curSpan = spans.back();
			prevSpanOff = curSpan.Offset(offset);
			nspans++; // read first again
		}

		std::list<Span>::iterator It = spans.begin();

		for(unsigned int spannumber = 1; spannumber <= nspans; spannumber++) {
			if (It == spans.end())
				curSpan = spans.front();						// closed CCurve - read first span again
			else
				curSpan = *It;

			{
				curSpanOff = curSpan.Offset(offset);
				curSpanOff.m_v.m_user_data = 0;
				if(kOffset.m_vertices.size() == 0) {
					kOffset.m_vertices.push_back(curSpanOff.m_p);
				}

				std::list<Point> pts;

				if(spannumber > 1) {
					// see if tangent
					double d = curSpanOff.m_p.Dist(prevSpanOff.m_v.m_p);
					if(d > TOLERANCE) {
						// see if offset spans intersect

						double cp = prevSpanOff.GetVector(1.0) ^ curSpanOff.GetVector(0.0);
						bool inters = (cp > 0 && direction == GEOFF_LEFT) || (cp < 0 && direction == GEOFF_RIGHT);

						if(inters) {
							prevSpanOff.Intersect(curSpanOff, pts);
						}

						if(pts.size() == 1) {
							// intersection - modify previous endpoint
							kOffset.m_vertices.back().m_p = pts.front();
						}
						else {
							// 0 or 2 intersections, add roll around (remove -ve loops in elimination function)
							kOffset.m_vertices.push_back(CVertex(RollDir, curSpanOff.m_p, curSpan.m_p, ROLL_AROUND));
						}
					}
				}

				// add span
				if (spannumber < m_vertices.size()) {
					curSpanOff.m_v.m_user_data = spannumber;
					kOffset.m_vertices.push_back(curSpanOff.m_v);
				}
				else if (pts.size() == 1)		// or replace the closed first span
					kOffset.m_vertices.front().m_p = pts.front();

			}

			It++;			
			prevSpanOff = curSpanOff;
		}		// end of main pre-offsetting loop


		// eliminate loops
		kOffset = eliminateLoops(kOffset, *this, offset, ret);

		return (ret == 0)?1 : 0;
	}


	static CCurve eliminateLoops(const CCurve& k , const CCurve& originalk, double offset, int& ret) {
		// a simple loop elimination routine based on first offset ideas in Peps
		// this needs extensive work for future
		// start point musn't disappear & only one valid offset is determined
		//
		// ret = 0 for ok
		// ret = 2 for impossible geometry
		
		Span sp0, sp1;
		CCurve ko;											// eliminated output

		std::list<CVertex>::const_iterator It = k.m_vertices.begin();
		bool start = true;

		while (It != k.m_vertices.end()) {
			bool clipped = false ;                                       // not in a clipped section (assumption with this simple method)

			sp0.m_p = It->m_p;
			It++;

			if (start)	{
				ko.append(CVertex(sp0.m_p, It->m_user_data));				// start point mustn't dissappear for this simple method
				start = false;
			}

			if (It != k.m_vertices.end()) {   // any more?
				std::list<CVertex>::const_iterator SaveIt = It;
				sp0.m_v = *It; // first span
				It++;

				std::list<CVertex>::const_iterator SaveIt1 = It;								// mark position AA		
				if (It != k.m_vertices.end()) {	// get the next but one span			
					sp1.m_p = It->m_p;
					It++;
					std::list<CVertex>::const_iterator SaveIt2 = It;					// mark position BB

					int fwdCount = 0;
					while (It != k.m_vertices.end()) {
						sp1.m_v = *It;
						It++;
			
						std::list<Point> pts;
						sp0.Intersect(sp1, pts);			// find span intersections
						if((pts.size() > 0) && (sp0.m_p.Dist(pts.front()) < TOLERANCE )) pts.clear();	// check that intersection is not at the start of the check span					
						if (pts.size() > 0) {

							if (pts.size() == 2) {
								// choose first intercept on sp0
								Span spd = sp0;
								spd.m_v.m_p = pts.front();
								double dd = spd.Length();

								spd.m_v.m_p = pts.back();
								if (dd > spd.Length())pts.pop_front();
								else pts.pop_back();

							}
							SaveIt = SaveIt1;

							clipped = true ;			// in a clipped section		
							if (DoesIntersInterfere(pts.front(), originalk, offset) == false) {
								sp0.m_v.m_p = pts.front();			// ok so truncate this span to the intersection
								clipped = false;		// end of clipped section
								break;
							}
							// no valid intersection found so carry on
						}
						sp1.m_p = sp1.m_v.m_p ;		// next
						SaveIt1 = SaveIt2;							// pos AA = BB
						SaveIt2 = It;								// mark 

						if ((It == k.m_vertices.end() || fwdCount++ > 25) && clipped == false) break;
					}
				}

				if(clipped) {
					ret = 2;	// still in a clipped section - error

					return ko;
				}

				ko.append(sp0.m_v);

				It = SaveIt;
			}
		}
		ret = 0;

		return ko; // no more spans - seems ok
	}


	static bool DoesIntersInterfere(const Point& pInt, const CCurve& k, double offset)  {
		// check that intersections don't interfere with the original CCurve 
		Span sp;
		Point dummy;
		std::list<CVertex>::const_iterator It = k.m_vertices.begin();
		sp.m_p = It->m_p;
		It++;

		offset = fabs(offset) - TOLERANCE;


		while (It != k.m_vertices.end()) {
			sp.m_v = *It;
			It++;

			// check for interference 
			if(sp.Dist(pInt) < offset) return true;
			sp.m_p = sp.m_v.m_p;
		}
		return false;	// intersection is ok
	}


