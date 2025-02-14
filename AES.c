#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include<stdlib.h>
#include<sys/stat.h>

#define Nk 4  // Number of 32-bit words in the key (4 for AES-128)
#define Nb 4  // Number of columns in the state
#define Nr 10 // Number of rounds for AES-128
#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 16

// S-Box
uint8_t S_BOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Inverse S-Box
uint8_t INV_S_BOX[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

//Pre-computed tables
uint32_t T0[256], T1[256], T2[256], T3[256];
uint32_t Tinv0[256], Tinv1[256], Tinv2[256], Tinv3[256];
// AES Rcon array for round constant
static const uint32_t RCON[11] = {
    0x00000000, 0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1b000000, 0x36000000
};

//GF(2^8) Multiplication
uint8_t gf8_mul(uint8_t a, uint8_t b) {
    uint8_t res = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) {
            res ^= a;
        }
        
        uint8_t hi_bit_set = a & 0x80;
        a <<= 1;
        if (hi_bit_set){
            a ^= 0x1b; //// 0000 0001 0001 1011
        } 
        b >>= 1;
    }
    return res;
}

// Precompute the T-Tables
/*
T0->02              
    01  .[S00 S01 S02 S03]    
    01
    03
*/
void precompute_tables() {
    for (int x = 0; x < 256; x++) {
        uint8_t s = S_BOX[x];
        T0[x] = (gf8_mul(s, 0x02) << 24) | (s << 16) | (s << 8) | gf8_mul(s, 0x03);
        T1[x] = (gf8_mul(s, 0x03) << 24) | (gf8_mul(s, 0x02) << 16) | (s << 8) | s;
        T2[x] = (s << 24) | (gf8_mul(s, 0x03) << 16) | (gf8_mul(s, 0x02) << 8) | s;
        T3[x] = (s << 24) | (s << 16) | (gf8_mul(s, 0x03) << 8) | gf8_mul(s, 0x02);
    }
}
void precompute_inverse_tables() {
    for (int x = 0; x < 256; x++) {
        uint8_t s = INV_S_BOX[x];
        Tinv0[x] = (gf8_mul(s, 0x0E) << 24) | (gf8_mul(s, 0x09) << 16) | (gf8_mul(s, 0x0D) << 8) | gf8_mul(s, 0x0B);
        Tinv1[x] = (gf8_mul(s, 0x0B) << 24) | (gf8_mul(s, 0x0E) << 16) | (gf8_mul(s, 0x09) << 8) | gf8_mul(s, 0x0D);
        Tinv2[x] = (gf8_mul(s, 0x0D) << 24) | (gf8_mul(s, 0x0B) << 16) | (gf8_mul(s, 0x0E) << 8) | gf8_mul(s, 0x09);
        Tinv3[x] = (gf8_mul(s, 0x09) << 24) | (gf8_mul(s, 0x0D) << 16) | (gf8_mul(s, 0x0B) << 8) | gf8_mul(s, 0x0E);
    }
}


// RotWord: Perform a left circular shift (rotate) by 8 bits
uint32_t RotWord(uint32_t word) {
    return (word << 8) | (word >> 24); // Rotate left by 8 bits
}

// SubWord: Apply the AES S-box to each byte of the 32-bit word
uint32_t SubWord(uint32_t word) {
    uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
        // Extract each byte and apply the S-box substitution
        uint8_t byte = (word >> (8 * (3 - i))) & 0xFF;
        result |= S_BOX[byte] << (8 * (3 - i));
    }
    return result;
}

// Key Expansion function
void KeyExpansion(const uint32_t *key, uint32_t *ekey) {
    uint32_t temp;

    // Copy the original key into the expanded key array
    for (int i = 0; i < 4; ++i) {
        ekey[i] = key[i];
    }

    // Perform key expansion
    for (int i = 4; i < 44; ++i) {
        temp = ekey[i - 1];

        
        if (i % 4 == 0) {
            temp = SubWord(RotWord(temp)) ^ RCON[i / 4];
        }

        ekey[i] = ekey[i - 4] ^ temp;
    }
}


void AddRoundKey(uint32_t *state, const uint32_t *roundKey, uint8_t round) {
    
//    printf("Round %d", round);
//        printf("\n");    
       
    // The round key is a 4-word block, so each round uses 4 consecutive uint32_t words
    uint32_t *key = (uint32_t *)roundKey + round * 4;

    // XOR each element of the state with the corresponding round key
    for (int i = 0; i < 4; ++i) {
//        printf("Previous block :%08x ", state[i]);
//        printf("\n");
//        printf("round key  :%08x ", key[i]);
//        printf("\n");

        state[i] ^= key[i];  // XOR the state element with the corresponding round key element

//         printf("result block :%08x ", state[i]);
//        printf("\n");
    }
}

// AES Encryption Function
void AES_Encrypt(uint32_t *state, uint32_t *output, const uint32_t *roundKeys) {

    // Initial AddRoundKey
    AddRoundKey(state, roundKeys,0);
    // printf("Encrypted output round 0: ");
    // for (int i = 0; i < 4; ++i) {
    //     printf("%02x ", state[i]);
    // }
    // printf("\n");
    // Main rounds
    for (int round = 1; round < Nr; ++round) {
        uint32_t tmp[Nb];
        for (int i = 0; i < Nb; ++i) {
            tmp[i] = T0[(state[i] >> 24)&0xFF] ^
                     T1[(state[(i + 1) % Nb] >> 16) & 0xFF] ^
                     T2[(state[(i + 2) % Nb] >> 8) & 0xFF] ^
                     T3[state[(i + 3) % Nb] & 0xFF];
                     
        }
        memcpy(state, tmp, Nb * sizeof(uint32_t));
        AddRoundKey(state,roundKeys,round);
        // printf("Encrypted output round %d: ",round);
        // for (int i = 0; i < 4; ++i) {
        //     printf("%02x ", state[i]);
            
        // }
        // printf("\n");
    }

    // Final round (without MixColumns)
    uint32_t tmp[Nb];
    for (int i = 0; i < Nb; ++i) {
        tmp[i] = (S_BOX[(state[i] >> 24)] << 24) |
                 (S_BOX[(state[(i + 1) % Nb] >> 16) & 0xFF] << 16) |
                 (S_BOX[(state[(i + 2) % Nb] >> 8) & 0xFF] << 8) |
                 S_BOX[state[(i + 3) % Nb] & 0xFF];
                 
    }
    memcpy(state, tmp, Nb * sizeof(uint32_t));
    AddRoundKey(state, roundKeys,10);

    // printf("Encrypted output round 10: ");
    // for (int i = 0; i < 4; ++i) {
    //     printf("%02x ", state[i]);
    // }
    // printf("\n");

    // Store state into output
 for (int i = 0; i < 4; ++i) {
        output[i] = state[i];
    }

}

void InverseMixColumnsKey(const uint32_t *words, uint32_t *result) {
    uint8_t bytes[4];

    for (int i = 0; i < 4; ++i) {
        // Extract the 4 bytes from the 32-bit key word
        bytes[0] = (words[i] >> 24) & 0xFF;
        bytes[1] = (words[i] >> 16) & 0xFF;
        bytes[2] = (words[i] >> 8) & 0xFF;
        bytes[3] = words[i] & 0xFF;

        // Perform the Inverse Mix Columns transformation
        result[i] = ((uint32_t)(gf8_mul(bytes[0], 0x0e) ^ gf8_mul(bytes[1], 0x0b) ^ gf8_mul(bytes[2], 0x0d) ^ gf8_mul(bytes[3], 0x09)) << 24) |
                    ((uint32_t)(gf8_mul(bytes[0], 0x09) ^ gf8_mul(bytes[1], 0x0e) ^ gf8_mul(bytes[2], 0x0b) ^ gf8_mul(bytes[3], 0x0d)) << 16) |
                    ((uint32_t)(gf8_mul(bytes[0], 0x0d) ^ gf8_mul(bytes[1], 0x09) ^ gf8_mul(bytes[2], 0x0e) ^ gf8_mul(bytes[3], 0x0b)) << 8) |
                    ((uint32_t)(gf8_mul(bytes[0], 0x0b) ^ gf8_mul(bytes[1], 0x0d) ^ gf8_mul(bytes[2], 0x09) ^ gf8_mul(bytes[3], 0x0e)));
    }
}

void PreprocessRoundKeys(const uint32_t *originalKeys, uint32_t *preprocessedKeys) {
    uint32_t temp[4]; // Temporary array to hold preprocessed words

    // Copy the original keys for round 0 and round Nr (no preprocessing needed for these rounds)
    for (int i = 0; i < Nb; ++i) {
        preprocessedKeys[0 * Nb + i] = originalKeys[0 * Nb + i];
        preprocessedKeys[Nr * Nb + i] = originalKeys[Nr * Nb + i];
    }

    // Preprocess keys for rounds 1 to Nr - 1
    for (int round = 1; round < Nr; ++round) {
        // Pass 4 words at a time to InverseMixColumnsKey
        InverseMixColumnsKey(&originalKeys[round * Nb], temp);

        // Copy the preprocessed result back to the appropriate position in preprocessedKeys
        for (int i = 0; i < Nb; ++i) {
            preprocessedKeys[round * Nb + i] = temp[i];
        }
    }
}



void AES_Decrypt(uint32_t *state, uint32_t *output, const uint32_t *processedKeys) {
    // AddRoundKey for the final round key (Nr round)
    AddRoundKey(state, processedKeys, Nr);

    // printf("Decrypted output round 10: ");
    // for (int i = 0; i < 4; ++i) {
    //     printf("%02x ", state[i]);
    // }
    // printf("\n");

    // Perform the decryption rounds in reverse order
    for (int round = Nr - 1; round > 0; --round) {
        uint32_t tmp[Nb];

        // Inverse ShiftRows and Inverse SubBytes (combined using precomputed tables)
        for (int i = 0; i < Nb; ++i) {
            tmp[i] = Tinv0[(state[i] >> 24) & 0xFF] ^
                     Tinv1[(state[(i + Nb - 1) % Nb] >> 16) & 0xFF] ^
                     Tinv2[(state[(i + Nb - 2) % Nb] >> 8) & 0xFF] ^
                     Tinv3[state[(i + Nb - 3) % Nb] & 0xFF];
        }
        memcpy(state, tmp, Nb * sizeof(uint32_t));

        // AddRoundKey for this round
        AddRoundKey(state, processedKeys, round);
        // printf("Decrypted output round %d: ",round);
        // for (int i = 0; i < 4; ++i) {
        //     printf("%02x ", state[i]);
            
        // }
        // printf("\n");
    }

    // Final Inverse ShiftRows and Inverse SubBytes round
    uint32_t tmp[Nb];
    for (int i = 0; i < Nb; ++i) {
         tmp[i] = (INV_S_BOX[(state[i] >> (24 - (0 * 8))) & 0xFF] << 24) |
             (INV_S_BOX[(state[(i + Nb - 1) % Nb] >> (24 - (1 * 8))) & 0xFF] << 16) |
             (INV_S_BOX[(state[(i + Nb - 2) % Nb] >> (24 - (2 * 8))) & 0xFF] << 8)  |
             (INV_S_BOX[(state[(i + Nb - 3) % Nb] >> (24 - (3 * 8))) & 0xFF]);
    }

    memcpy(state, tmp, Nb * sizeof(uint32_t));
    // Final AddRoundKey for round 0
    AddRoundKey(state, processedKeys, 0);

    // Copy the result to output
    memcpy(output, state, Nb * sizeof(uint32_t));
    // printf("Final decrypted output:\n");
    // for (int i = 0; i < 4; ++i) {
    //     printf("%08x ", state[i]);
    // }
    // printf("\n");
}


unsigned char* load_file(const char *fn, int *len)
{
	struct stat info={0};
	int ret=stat(fn, &info);
	if(ret)//inaccessible
		return 0;
	FILE *fsrc=fopen(fn, "rb");
	if(!fsrc)//inaccessible
		return 0;
	unsigned char * data=(unsigned char *)malloc(info.st_size);//remember to free(data) at the end
	if(!data)//out of memory
	{
		exit(1);
		return 0;
	}
	size_t nread=fread(data, 1, info.st_size, fsrc);
	fclose(fsrc);
	*len=(int)nread;
	return data;
}

//Use this code to write files:
int save_file(const char *fn, unsigned char *data, int len)
{
	FILE *fdst=fopen(fn, "wb");
	if(!fdst)
		return 0;
	fwrite(data, 1, len, fdst);
	fclose(fdst);
	return 1;
}

int main(int argc, char *argv[]) 
{

    //Precompute the lookup tables
    precompute_tables(T0, T1, T2, T3);
    precompute_inverse_tables(Tinv0, Tinv1, Tinv2, Tinv3);
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <keyfile> <inputfile> <outputfile>\n", argv[0]);
        return 1;
    }

    // Load the key from a file
    int key_len;
    unsigned char *key_data = load_file(argv[2], &key_len);
    if (!key_data || key_len != AES_KEY_SIZE) {
        fprintf(stderr, "Failed to load the key or key size is incorrect(must be 16 bytes)\n");
        return 1;
    }

    // Load the data from a file
    int data_len;
    unsigned char *data = load_file(argv[3], &data_len);
    if (!data) {
        fprintf(stderr, "Failed to load the data\n");
        free(key_data);
        return 1;
    }

    // Ensure data length is a multiple of 16 (128 bits)
    if (data_len % AES_BLOCK_SIZE != 0) {
        fprintf(stderr, "Data length must be a multiple of 16 bytes\n");
        free(data);
        free(key_data);
        return 1;
    }

    // Prepare to save encrypted data
    unsigned char *encrypted_data = (unsigned char *)malloc(data_len);
    if (!encrypted_data) {
        fprintf(stderr, "Memory allocation for encrypted data failed\n");
        free(data);
        free(key_data);
        return 1;
    }

    // Convert the key to a uint32_t array of 4 elements
    uint32_t key[4];
    for (int i = 0; i < 4; ++i) {
        key[i] = ((uint32_t)key_data[i * 4] << 24) |
                 ((uint32_t)key_data[i * 4 + 1] << 16) |
                 ((uint32_t)key_data[i * 4 + 2] << 8) |
                ((uint32_t)key_data[i * 4 + 3]);
    }
    free(key_data); // Free the raw key data after conversion
    
    // Key expansion
    uint32_t roundKeys[44];
    uint32_t processedKeys[44];

    KeyExpansion(key, roundKeys);
    PreprocessRoundKeys(roundKeys, processedKeys);

    // Encrypt each 128-bit block
    for (int i = 0; i < data_len; i += AES_BLOCK_SIZE) {
        uint32_t state[4];      // AES state (128 bits = 4 x 32-bit words)
        uint32_t output[4];     // Output buffer for encryption/decryption

        // Load 16 bytes into state as 4 x 32-bit words
        for (int j = 0; j < 4; ++j) {
            state[j] = ((uint32_t)data[i + j * 4]) << 24 |
                       ((uint32_t)data[i + j * 4 + 1]) << 16 |
                       ((uint32_t)data[i + j * 4 + 2]) << 8 |
                       ((uint32_t)data[i + j * 4 + 3]);
        }

        // Perform encryption or decryption
        if(argv[1][0]=='e'){
            AES_Encrypt(state, output, roundKeys);
        }else{
            AES_Decrypt(state, output, processedKeys);
        }

        // Store the output back into the output buffer
        for (int j = 0; j < 4; ++j) {
            encrypted_data[i + j * 4] = (output[j] >> 24) & 0xFF;
            encrypted_data[i + j * 4 + 1] = (output[j] >> 16) & 0xFF;
            encrypted_data[i + j * 4 + 2] = (output[j] >> 8) & 0xFF;
            encrypted_data[i + j * 4 + 3] = output[j] & 0xFF;
        }
    }

    // Save the encrypted data to a file
    if (!save_file(argv[4], encrypted_data, data_len)) {
        fprintf(stderr, "Failed to save the encrypted data\n");
        free(data);
        free(encrypted_data);
        return 1;
    }

    // Cleanup
    free(data);
    free(encrypted_data);
    return 0;
}

/*

int main() {

    uint32_t input [4] = {0x01234567,0x89ABCDEF,
	0xFEDCBA98,0x76543210};


	uint32_t key[4] = {0x0F1571C9,0x47D9E859,0x1CB7ADD6,0xAF7F6798};
    
    // Buffer for round keys
    uint32_t roundKeys[44];
    uint32_t processedKeys[44];

    uint32_t EncOutput [4];
    uint32_t DecOutput [4];
    // Perform key expansion
    KeyExpansion(key, roundKeys);
    PreprocessRoundKeys(roundKeys, processedKeys);

    precompute_tables(T0, T1, T2, T3);
    precompute_inverse_tables(Tinv0, Tinv1, Tinv2, Tinv3);
    
    AES_Encrypt(input, EncOutput, roundKeys);

    printf("Expanded key:\n");
        for (int i = 0; i < 44; ++i) {
            printf("roundKeys[%d] = %08x\n", i, roundKeys[i]);
        }


   
    printf("Encrypted output: ");
    for (int i = 0; i < 4; ++i) {
        printf("%04x ", EncOutput[i]);
    }
    printf("\n");

    AES_Decrypt(EncOutput, DecOutput, processedKeys);
    printf("Decrypted output: ");
    for (int i = 0; i < 4; ++i) {
        printf("%04x ", DecOutput[i]);
    }
    printf("\n");


// Testing the inverse mix columns

    uint32_t words[4] = {0x9a1635bc, 0x6916971c, 0x67c215df, 0x5b6b1e56};
    uint32_t result[4]; // Pre-allocated array for the result

    InverseMixColumnsKey(words, result);

    for (int i = 0; i < 4; ++i) {
        printf("Word %d: 0x%08x\n", i, result[i]);
    }


    return 0;
}

*/