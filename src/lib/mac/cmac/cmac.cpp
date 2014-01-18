/*
* CMAC
* (C) 1999-2007,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <botan/cmac.h>
#include <botan/loadstor.h>
#include <botan/internal/xor_buf.h>

namespace Botan {

/*
* Perform CMAC's multiplication in GF(2^n)
*/
secure_vector<byte> CMAC::poly_double(const secure_vector<byte>& in)
   {
   const byte polynomial = (in.size() == 16) ? 0x87 : 0x1B;

   const byte poly_xor = (in[0] & 0x80) ? polynomial : 0;

   secure_vector<byte> out = in;

   byte carry = 0;
   for(size_t i = out.size(); i != 0; --i)
      {
      byte temp = out[i-1];
      out[i-1] = (temp << 1) | carry;
      carry = (temp >> 7);
      }

   out[out.size()-1] ^= poly_xor;

   return out;
   }

/*
* Update an CMAC Calculation
*/
void CMAC::add_data(const byte input[], size_t length)
   {
   buffer_insert(m_buffer, m_position, input, length);
   if(m_position + length > output_length())
      {
      xor_buf(m_state, m_buffer, output_length());
      m_cipher->encrypt(m_state);
      input += (output_length() - m_position);
      length -= (output_length() - m_position);
      while(length > output_length())
         {
         xor_buf(m_state, input, output_length());
         m_cipher->encrypt(m_state);
         input += output_length();
         length -= output_length();
         }
      copy_mem(&m_buffer[0], input, length);
      m_position = 0;
      }
   m_position += length;
   }

/*
* Finalize an CMAC Calculation
*/
void CMAC::final_result(byte mac[])
   {
   xor_buf(m_state, m_buffer, m_position);

   if(m_position == output_length())
      {
      xor_buf(m_state, m_B, output_length());
      }
   else
      {
      m_state[m_position] ^= 0x80;
      xor_buf(m_state, m_P, output_length());
      }

   m_cipher->encrypt(m_state);

   for(size_t i = 0; i != output_length(); ++i)
      mac[i] = m_state[i];

   zeroise(m_state);
   zeroise(m_buffer);
   m_position = 0;
   }

/*
* CMAC Key Schedule
*/
void CMAC::key_schedule(const byte key[], size_t length)
   {
   clear();
   m_cipher->set_key(key, length);
   m_cipher->encrypt(m_B);
   m_B = poly_double(m_B);
   m_P = poly_double(m_B);
   }

/*
* Clear memory of sensitive data
*/
void CMAC::clear()
   {
   m_cipher->clear();
   zeroise(m_state);
   zeroise(m_buffer);
   zeroise(m_B);
   zeroise(m_P);
   m_position = 0;
   }

/*
* Return the name of this type
*/
std::string CMAC::name() const
   {
   return "CMAC(" + m_cipher->name() + ")";
   }

/*
* Return a clone of this object
*/
MessageAuthenticationCode* CMAC::clone() const
   {
   return new CMAC(m_cipher->clone());
   }

/*
* CMAC Constructor
*/
CMAC::CMAC(BlockCipher* cipher) : m_cipher(cipher)
   {
   if(m_cipher->block_size() != 8 && m_cipher->block_size() != 16)
      throw Invalid_Argument("CMAC cannot use the cipher " + m_cipher->name());

   m_state.resize(output_length());
   m_buffer.resize(output_length());
   m_B.resize(output_length());
   m_P.resize(output_length());
   m_position = 0;
   }

}
