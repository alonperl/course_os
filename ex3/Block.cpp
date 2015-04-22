
Block::Block(arguments)
{

}

Block::~Block()
{

}

/**
 * @return the block's Id
 */
int Block::getId(void)
{
	return _blockId;
}

/**
 * @return the block's timestamp
 */
int Block::getTimestamp(void)
{
	return _timestamp;
}

/**
 * @return the block's height
 */
int Block::getHeight(void)
{
	return _height;
}

/**
 * @return the block's hash data
 */
string Block::getHashData(void)
{
	return _hashData;
}

/**
 * @return the blocks father
 */
*Block Block::getPrevBlock(void)
{
	return _prevBlock;
}
