/*
 * interface class definition
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/
/*
example :
DeclareInterface(IBar)
   virtual int GetBarData() const = 0;
   virtual void SetBarData(int nData) = 0;
EndInterface

*/
#ifdef __cplusplus

	#ifndef __H_INTERFACE_MY___
	#define __H_INTERFACE_MY___

		#define Interface class

		#define DeclareInterface(name) Interface name { \
				  public: \
				  virtual ~name() {}
/*
		#define DeclareBasedInterface(name, base) Interface name :
				public base { \
				   public: \
				   virtual ~name() {}
*/

		#define EndInterface };

		#define implements public

	#endif //__H_INTERFACE_MY___

#endif //__cplusplus
