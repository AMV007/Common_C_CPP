#include "stdafx.h"
#include "..\\IP\\c_smtp.h"
#include "..\\IP\\c_pop3.h"

#include <iostream>
#include <ctype.h>

void TestSMTP()
{

#if 0
	char server[] = "smtp.rambler.ru";
	char from[] = "amv008@rambler.ru";
	char to[] = "m_akristinii@rambler.ru";	
	char login[] = "amv008";
	char pass[] = "maximum";
#else		
	char server[] = "smtp.mail.ru";
	char from[] = "m_akristinii@mail.ru";
	char to[] = "m_akristinii@mail.ru";	
	char login[] = "m_akristinii";
	char pass[] = "maximum1";
#endif

	char subject[] = "GXS SMTP Test Message";
	char body[] = "Test test test... hohohohoho, 192.168.10.4 \n dsssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssáëà áëà áëà áëà ïðèâåò äîðîãóøà\n";

	c_smtp * xx = new c_smtp();	
	do
	{
		if(!xx->ConnectEClient(server,login,pass)) break;
		printf("%s\n",xx->reply_buf);		

		if(false)
		{
			// íåñêîëüêèìè ôóíêöèÿìè, çàòî áåç òåìû
			//if(!xx->SMTPRSet()) break;
			//printf("%s\n",xx->reply_buf);
			if(!xx->SMTPMailFrom((const char *)from)) break;	
			printf("%s\n",xx->reply_buf);
			if(!xx->SMTPRcptTo((const char *)to)) break;	
			printf("%s\n",xx->reply_buf);
			if(!xx->SMTPData((const char *)body, (int)strlen(body))) break;	
			printf("%s\n",xx->reply_buf);
		}
		else
		{ // îäíîé ôóíêöèåé, íî ñ òåìîé			
			if(!xx->SMTPSendMessage(to, from, subject, body))break;
			printf("%s\n",xx->reply_buf);
		}

		if(!xx->SMTPLogout()) break;	
		printf("%s\n",xx->reply_buf);
	}while(false);
	if(xx->GetLocalErrorSMTP()!=0) printf("Error, %s",xx->reply_buf);
	xx->SMTPLogout();		
	delete xx;
}

void TestPOP3()
{
	char server[] = "pop.rambler.ru";
	char login[] = "m_akristinii";
	char pass[] = "maximum1";

	c_pop3 * xx = new c_pop3();	
	do
	{		
		if(!xx->ConnectEClient(server,login,pass)) break;
		printf("%s\n",xx->reply_buf);			

		int MsgCount=0;
		int MsgSize =0;
		int MsgSizes[32];
		if(!xx->POP3List(&MsgCount, &MsgSize,MsgSizes,32)) break;
		printf("%s\n",xx->reply_buf);		
		
		//if(!xx->POP3Delete(1)) break;
		//printf("%s\n",xx->reply_buf);

		char TempBuf[1024*16];
		if(!xx->POP3Retr(1,TempBuf,sizeof(TempBuf))) break;
		printf("%s\n",xx->reply_buf);		
		printf("Message: %s\n",TempBuf);		
	}while(false);
	if(xx->GetLocalErrorPOP3()!=0) printf("Error, %s",xx->reply_buf);
	xx->POP3Logout();	
	delete xx;
}

void RunSMTPPOP3Test()
{
	TestSMTP();
	TestPOP3();
	system("pause");
}