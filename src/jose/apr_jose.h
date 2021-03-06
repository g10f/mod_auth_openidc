/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/***************************************************************************
 * Copyright (C) 2013-2014 Ping Identity Corporation
 * All rights reserved.
 *
 * For further information please contact:
 *
 *      Ping Identity Corporation
 *      1099 18th St Suite 2950
 *      Denver, CO 80202
 *      303.468.2900
 *      http://www.pingidentity.com
 *
 * DISCLAIMER OF WARRANTIES:
 *
 * THE SOFTWARE PROVIDED HEREUNDER IS PROVIDED ON AN "AS IS" BASIS, WITHOUT
 * ANY WARRANTIES OR REPRESENTATIONS EXPRESS, IMPLIED OR STATUTORY; INCLUDING,
 * WITHOUT LIMITATION, WARRANTIES OF QUALITY, PERFORMANCE, NONINFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  NOR ARE THERE ANY
 * WARRANTIES CREATED BY A COURSE OR DEALING, COURSE OF PERFORMANCE OR TRADE
 * USAGE.  FURTHERMORE, THERE ARE NO WARRANTIES THAT THE SOFTWARE WILL MEET
 * YOUR NEEDS OR BE FREE FROM ERRORS, OR THAT THE OPERATION OF THE SOFTWARE
 * WILL BE UNINTERRUPTED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * JSON Web Tokens Signing and Encryption
 *
 * @Author: Hans Zandbelt - hzandbelt@pingidentity.com
 */

#ifndef _APR_JOSE_H_
#define _APR_JOSE_H_

#include "apr_pools.h"
#include "apr_time.h"
#include "apr_tables.h"
#include "apr_hash.h"
#include "apr_strings.h"

#include "jansson.h"

#define APR_JWT_CLAIM_TIME_EMPTY -1

/*
 * JSON Web Token handling
 */

/* a parsed JWT "element", header or payload */
typedef struct apr_jwt_value_t {
	/* parsed JSON struct representation */
	json_t *json;
	/* string representation */
	char *str;
} apr_jwt_value_t;

/* a parsed JWT header */
typedef struct apr_jwt_header_t {
	/* parsed header value */
	apr_jwt_value_t value;
	/* JWT "alg" claim value; signing algorithm */
	char *alg;
	/* JWT "kid" claim value; key identifier */
	char *kid;
	/* JWT "enc" claim value; encryption algorithm */
	char *enc;
} apr_jwt_header_t;

/* parsed JWT payload */
typedef struct apr_jwt_payload_t {
	/* parsed payload value */
	apr_jwt_value_t value;
	/* JWT "iss" claim value; JWT issuer */
	char *iss;
	/* JWT "sub" claim value; subject/principal */
	char *sub;
	/* parsed JWT "exp" claim value; token expiry */
	apr_time_t exp;
	/* parsed JWT "iat" claim value; issued-at timestamp */
	apr_time_t iat;
} apr_jwt_payload_t;

/* parsed JWT signature */
typedef struct apr_jwt_signature_t {
	/* raw (base64url-decoded) signature value */
	unsigned char *bytes;
	/* length of the raw signature value */
	int length;
} apr_jwt_signature_t;

/* parsed JWT */
typedef struct apr_jwt_t {
	/* parsed JWT header */
	apr_jwt_header_t header;
	/* parsed JWT payload */
	apr_jwt_payload_t payload;
	/* decoded JWT signature */
	apr_jwt_signature_t signature;
	/* base64url-encoded header+payload (for signature verification purposes) */
	char *message;
} apr_jwt_t;

/* helpers */
typedef apr_byte_t (*apr_jose_is_supported_function_t)(apr_pool_t *, const char *);
apr_byte_t apr_jwt_array_has_string(apr_array_header_t *haystack,
		const char *needle);
int apr_jwt_base64url_encode(apr_pool_t *pool, char **dst, const char *src,
		int src_len, int padding);
int apr_jwt_base64url_decode(apr_pool_t *pool, char **dst, const char *src,
		int padding);
const char *apr_jwt_header_to_string(apr_pool_t *pool, const char *s_json);

/* return a string claim value from a JSON Web Token */
apr_byte_t apr_jwt_get_string(apr_pool_t *pool, apr_jwt_value_t *value,
		const char *claim_name, char **result);
/* parse a string in to a JSON Web Token struct */
apr_byte_t apr_jwt_parse(apr_pool_t *pool, const char *s_json,
		apr_jwt_t **j_jwt, apr_hash_t *private_keys, const char *shared_key);
/* destroy resources allocated for JWT */
void apr_jwt_destroy(apr_jwt_t *);

/* exported for the purpose of the test suite */
apr_array_header_t *apr_jwt_compact_deserialize(apr_pool_t *pool, const char *str);
apr_byte_t apr_jwt_parse_header(apr_pool_t *pool, const char *s_header, apr_jwt_header_t *header);

/*
 * JSON Web Key handling
 */

/* JWK key type */
typedef enum apr_jwk_type_e {
	/* RSA JWT key type */
	APR_JWK_KEY_RSA,
	/* EC JWT key type */
	APR_JWK_KEY_EC,
} apr_jwk_type_e;

/* parsed RSA JWK key */
typedef struct apr_jwk_key_rsa_t {
	/* (binary) RSA modulus */
	unsigned char *modulus;
	/* length of the binary RSA modulus */
	int modulus_len;
	/* (binary) RSA exponent */
	unsigned char *exponent;
	/* length of the binary RSA exponent */
	int exponent_len;
	/* (binary) RSA private exponent */
	unsigned char *private_exponent;
	/* length of the binary private RSA exponent */
	int private_exponent_len;
} apr_jwk_key_rsa_t;

/* parsed EC JWK key */
typedef struct apr_jwk_key_ec_t {
	/* x */
	unsigned char *x;
	/* length of x */
	int x_len;
	/* y */
	unsigned char *y;
	/* length of y */
	int y_len;
} apr_jwk_key_ec_t;

/* parsed JWK key */
typedef struct apr_jwk_t {
	/* parsed JWK/JSON value */
	apr_jwt_value_t value;
	/* type of JWK key */
	apr_jwk_type_e type;
	/* union/pointer to parsed JWK key */
	union {
		apr_jwk_key_rsa_t *rsa;
		apr_jwk_key_ec_t *ec;
	} key;
} apr_jwk_t;

/* parse a JSON representation in to a JSON Web Key struct (also storing the string representation */
apr_byte_t apr_jwk_parse_json(apr_pool_t *pool, json_t *j_json,
		const char *s_json, apr_jwk_t **j_jwk);
/* convert the RSA public key in a PEM formatted file with an X.509 cert in to an RSA JWK */
apr_byte_t apr_jwk_x509_to_rsa_jwk(apr_pool_t *pool, const char *filename, char **jwk, char**kid);
/* convert the RSA private key in a PEM formatted file in to an RSA JWK */
apr_byte_t apr_jwk_private_key_to_rsa_jwk(apr_pool_t *pool, const char *filename, char **jwk, char**kid);

/*
 * JSON Web Token Signature handling
 */

/* return all supported signing algorithms */
apr_array_header_t *apr_jws_supported_algorithms(apr_pool_t *pool);
/* check if the provided signing algorithm is supported */
apr_byte_t apr_jws_algorithm_is_supported(apr_pool_t *pool, const char *alg);

/* check if the signature on a JWT is of type HMAC */
apr_byte_t apr_jws_signature_is_hmac(apr_pool_t *pool, apr_jwt_t *jwt);
/* check if the signature on a JWT is of type RSA */
apr_byte_t apr_jws_signature_is_rsa(apr_pool_t *pool, apr_jwt_t *jwt);
/* check if the signature on a JWT is of type Elliptic Curve */
apr_byte_t apr_jws_signature_is_ec(apr_pool_t *pool, apr_jwt_t *jwt);
/* verify the HMAC signature on a JWT */
apr_byte_t apr_jws_verify_hmac(apr_pool_t *pool, apr_jwt_t *jwt,
		const char *key, const unsigned int key_len);
/* verify the RSA signature on a JWT */
apr_byte_t apr_jws_verify_rsa(apr_pool_t *pool, apr_jwt_t *jwt, apr_jwk_t *jwk);
/* verify the Elliptic Curve signature on a JWT */
apr_byte_t apr_jws_verify_ec(apr_pool_t *pool, apr_jwt_t *jwt, apr_jwk_t *jwk);

/* hash byte sequence */
apr_byte_t apr_jws_hash_bytes(apr_pool_t *pool, const char *s_digest,
		const unsigned char *input, unsigned int input_len, unsigned char **output, unsigned int *output_len);
/* hash a string */
apr_byte_t apr_jws_hash_string(apr_pool_t *pool, const char *alg,
		const char *msg, char **hash, unsigned int *hash_len);
/* length of hash */
int apr_jws_hash_length(const char *alg);

/*
 * JSON Web Token Encryption handling
 */

/* return all supported content encryption key algorithms */
apr_array_header_t *apr_jwe_supported_algorithms(apr_pool_t *pool);
/* check if the provided content encryption key algorithm is supported */
apr_byte_t apr_jwe_algorithm_is_supported(apr_pool_t *pool, const char *alg);
/* return all supported encryption algorithms */
apr_array_header_t *apr_jwe_supported_encryptions(apr_pool_t *pool);
/* check if the provided encryption algorithm is supported */
apr_byte_t apr_jwe_encryption_is_supported(apr_pool_t *pool, const char *enc);

apr_byte_t apr_jwe_is_encrypted_jwt(apr_pool_t *pool, apr_jwt_header_t *hdr);
apr_byte_t apr_jwe_decrypt_jwt(apr_pool_t *pool, apr_jwt_header_t *header, apr_array_header_t *unpacked, apr_hash_t *private_keys, const char *shared_key, char **decrypted);

#endif /* _APR_JOSE_H_ */
