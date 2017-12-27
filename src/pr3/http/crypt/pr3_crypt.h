#ifndef pr3_crypt_h
#define pr3_crypt_h


#include <stdio.h>

#include "../../../shared/vector.h"
#include "pr3_aes.h"


typedef struct cryptor {
	vector iv;
	AESKey AES;
} cryptor;


void cryptorInit(cryptor *crypt);
void cryptorSetIV(cryptor *crypt, const char *str, const size_t strLength);
void cryptorDecrypt(cryptor *crypt, const char *str, const size_t strLength, char **outStr, size_t *outLength);
void cryptorRemove(cryptor *crypt);


#endif

/*
Here's how we decrypt something with PR3.
Notes:
"stringToBinary" just calls our base64Decode function. The
vector you pass into ours holds the return value.


var crypt:Encrypt = new Encrypt();
crypt.SetKey("012345678910ABCD");
crypt.SetIV(storedProcID);
crypt.decrypt(storedProcedureName);


public function SetKey(param1:String):void{
	var _loc2_:ByteArray = new ByteArray();
	_loc2_.writeUTFBytes(param1);
	var _loc3_:ZeroPad = new ZeroPad();
	var _loc4_:AESKey = new AESKey(_loc2_);
	this.Encryption = new CBCMode(_loc4_,_loc3_);
}

public function SetIV(param1:String):void{
	var _loc2_:ByteArray = this.stringToBinary(param1);
	IVMode(this.Encryption).IV = _loc2_;
}

public function decrypt(param1:String):String{
	var _loc2_:ByteArray = this.stringToBinary(param1);
	this.Encryption.decrypt(_loc2_);
	_loc2_.position = 0;
	var _loc3_:String = _loc2_.readUTFBytes(_loc2_.bytesAvailable);
	return _loc3_;
}
*/