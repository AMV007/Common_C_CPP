#include "stdafx.h"
#include "..\\CRYPT\\COMMON\\havege.h"
#include "..\\CRYPT\\PUBLIC_KEY\\rsa.h"

#ifdef _MSC_VER
#pragma warning( disable : 4996) // can't something do with this warning
#endif

#define KEY_SIZE 1024
#define EXPONENT 65537

int GenKey(rsa_context &rsa)
{
	havege_state hs;
	//rsa_context rsa;

	printf( "\n  . Seeding the random number generator..." );
	fflush( stdout );

	havege_init( &hs );

	printf( " ok\n  . Generating the RSA key [ %d-bit ]...", KEY_SIZE );
	fflush( stdout );

	rsa_init( &rsa, RSA_PKCS_V15, 0, havege_rand, &hs );    

	int ret=0;
	if( (ret=rsa_gen_key( &rsa, KEY_SIZE, EXPONENT ) ) != 0 )
	{
		printf( " failed\n  ! rsa_gen_key returned %d\n\n", ret );
		return ret;
	}

	// write publick and provate keys into file
	FILE *fpub  = NULL;	

	printf( " ok\n  . Exporting the public  key in rsa_pub.txt...." );
	fflush( stdout );

	if( ( fpub = fopen( "rsa_pub.txt", "wb+" ) ) != NULL )
	{        
		if( ( ret = mpi_write_file( "N = ", &rsa.N, 16, fpub ) ) != 0 ||
			( ret = mpi_write_file( "E = ", &rsa.E, 16, fpub ) ) != 0 )
		{
			printf( " failed\n  ! mpi_write_file returned %d\n\n", ret );	
			return ret;
		}
	} else printf( " failed\n  ! could not open rsa_pub.txt for writing\n\n" );



	printf( " ok\n  . Exporting the private key in rsa_priv.txt..." );
	fflush( stdout );

	FILE *fpriv = NULL; 
	if( ( fpriv = fopen( "rsa_priv.txt", "wb+" ) ) != NULL )
	{

		if( ( ret = mpi_write_file( "N = " , &rsa.N , 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "E = " , &rsa.E , 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "D = " , &rsa.D , 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "P = " , &rsa.P , 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "Q = " , &rsa.Q , 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "DP = ", &rsa.DP, 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "DQ = ", &rsa.DQ, 16, fpriv ) ) != 0 ||
			( ret = mpi_write_file( "QP = ", &rsa.QP, 16, fpriv ) ) != 0 )
		{
			printf( " failed\n  ! mpi_write_file returned %d\n\n", ret );
			return ret;
		} 
	}else printf( " failed\n  ! could not open rsa_priv.txt for writing\n" );
	if(ret==0)printf( " ok\n\n" ); 

	return ret;
}

void Encrypt(rsa_context rsa, char * Data)
{

}

int rsatest()
{	
	rsa_context rsa;
	if(GenKey(rsa)==0)
	{

	}
	return 1;
}