We changed the functions bb_read(), bb_write() in the file bbfs.c.
We used a character cipher, by doing a bitwise XOR of the character, with another,
which is chosen from a key of 16 characters as:
c_i = p_i ^ k_(i%16); 
where,
	c_i is the (i)th character in the encrypted text
	p_i is the (i)th character in the non-encrypted text
	k_i is the (i)th character in the key

The code we wrote can be found by searching for the comment "ADDED" in the file.
'key' has been defined as a global array of characters