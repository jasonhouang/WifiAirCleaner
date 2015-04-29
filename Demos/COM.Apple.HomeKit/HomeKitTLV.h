#ifndef __HOMEKITTLV_h__
#define __HOMEKITTLV_h__

/*! @group      WACTLVConstants
    @abstract   Constants for element IDs, keys, etc.
*/

// [Integer] Method to use for pairing.
#define kTLVType_Method                 0x00

// [String] Username for authentication.
#define kTLVType_User                   0x01

// [bytes] 16+ bytes of random salt.
#define kTLVType_Salt                   0x02

// [bytes] Curve25519 or SRP public key. If > 255 bytes, it must be split across mutiple, contiguous items.
#define kTLVType_PublicKey              0x03

// [bytes] ED25519 or SRP proof. If > 255 bytes, it must be split across mutiple, contiguous items.
#define kTLVType_Proof                  0x04

// [bytes] Encrypted TLV8 data.
#define kTLVType_EncryptedData          0x05

// [bytes] Authentication tag for encrypted data.
#define kTLVType_AuthTag                0x06 

// [Integer] State of the pairing process. 1 = M1, 2 = M2, etc
#define kTLVType_State                  0x07

// [Integer] Status code.
#define kTLVType_Status                 0x08

// [Integer] Seconds to delay until retrying password.
#define kTLVType_RetryDelay             0x09

// [bytes] X.509 Certificate.
#define kTLVType_Certificate            0x0A

// [bytes] MFi auth IC signature.
#define kTLVType_MFiProof               0x0B

// [BOOL] Boolean value (0 or 1) that indicates if the controller is an admin.
#define kTLVType_Admin                  0x0C

// [null] Zero-length
#define kWACTLV_Separator               0x0D

#define kHATLV_MaxStringSize           	255
#define kHATLV_TypeLengthSize          	2


// Success, This is not normally included in a message. Absence of a status item implies seccess
#define kTLVStatus_NoErr   				0x00

// Generic error to handle unexcepted errors
#define kTLVStatus_UnknowErr   			0x01

// Password or signature verification failed
#define kTLVStatus_AuthenticationErr   	0x02

// There have been more than 10 authentication attempts and accessory must be physically reset
#define kTLVStatus_TooManyAttemptsErr   0x03

// Peer is not paired
#define kTLVStatus_UnknowPeerErr   		0x04

// Server cannot accept any more pairings
#define kTLVStatus_MaxPeerErr   		0x05

// Server reached its maximum nember of authentication attempts
#define kTLVStatus_MaxTriesErr   		0x06


#endif // __WACTLV_h__

