// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: hashes.cc,v 1.1 2001/03/06 07:15:29 jgg Exp $
/* ######################################################################

   Hashes - Simple wrapper around the hash functions
   
   This is just used to make building the methods simpler, this is the
   only interface required..
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <config.h>

#include <apt-pkg/hashes.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/macros.h>

#include <unistd.h>
#include <string>
#include <iostream>
									/*}}}*/

const char* HashString::_SupportedHashes[] = 
{
   "SHA512", "SHA256", "SHA1", "MD5Sum", NULL
};

HashString::HashString()
{
}

HashString::HashString(string Type, string Hash) : Type(Type), Hash(Hash)
{
}

HashString::HashString(string StringedHash)				/*{{{*/
{
   // legacy: md5sum without "MD5Sum:" prefix
   if (StringedHash.find(":") == string::npos && StringedHash.size() == 32)
   {
      Type = "MD5Sum";
      Hash = StringedHash;
      return;
   }
   string::size_type pos = StringedHash.find(":");
   Type = StringedHash.substr(0,pos);
   Hash = StringedHash.substr(pos+1, StringedHash.size() - pos);

   if(_config->FindB("Debug::Hashes",false) == true)
      std::clog << "HashString(string): " << Type << " : " << Hash << std::endl;
}
									/*}}}*/
bool HashString::VerifyFile(string filename) const			/*{{{*/
{
   string fileHash;

   FileFd Fd(filename, FileFd::ReadOnly);
   if(Type == "MD5Sum")
   {
      MD5Summation MD5;
      MD5.AddFD(Fd.Fd(), Fd.Size());
      fileHash = (string)MD5.Result();
   }
   else if (Type == "SHA1")
   {
      SHA1Summation SHA1;
      SHA1.AddFD(Fd.Fd(), Fd.Size());
      fileHash = (string)SHA1.Result();
   }
   else if (Type == "SHA256")
   {
      SHA256Summation SHA256;
      SHA256.AddFD(Fd.Fd(), Fd.Size());
      fileHash = (string)SHA256.Result();
   }
   else if (Type == "SHA512")
   {
      SHA512Summation SHA512;
      SHA512.AddFD(Fd.Fd(), Fd.Size());
      fileHash = (string)SHA512.Result();
   }
   Fd.Close();

   if(_config->FindB("Debug::Hashes",false) == true)
      std::clog << "HashString::VerifyFile: got: " << fileHash << " expected: " << toStr() << std::endl;

   return (fileHash == Hash);
}
									/*}}}*/
const char** HashString::SupportedHashes()
{
   return _SupportedHashes;
}

bool HashString::empty() const
{
   return (Type.empty() || Hash.empty());
}

string HashString::toStr() const
{
   return Type+string(":")+Hash;
}

// Hashes::AddFD - Add the contents of the FD				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Hashes::AddFD(int const Fd,unsigned long Size, bool const addMD5,
		   bool const addSHA1, bool const addSHA256, bool const addSHA512)
{
   unsigned char Buf[64*64];
   int Res = 0;
   int ToEOF = (Size == 0);
   while (Size != 0 || ToEOF)
   {
      unsigned n = sizeof(Buf);
      if (!ToEOF) n = min(Size,(unsigned long)n);
      Res = read(Fd,Buf,n);
      if (Res < 0 || (!ToEOF && (unsigned) Res != n)) // error, or short read
	 return false;
      if (ToEOF && Res == 0) // EOF
	 break;
      Size -= Res;
      if (addMD5 == true)
	 MD5.Add(Buf,Res);
      if (addSHA1 == true)
	 SHA1.Add(Buf,Res);
      if (addSHA256 == true)
	 SHA256.Add(Buf,Res);
      if (addSHA512 == true)
	 SHA512.Add(Buf,Res);
   }
   return true;
}
									/*}}}*/

