/*
* Copyright (c) 2014, 百年千岁
* All rights reserved.
* 
* 文件名称：vxRTSPMemory.inl
* 创建日期：2014年7月25日
* 文件标识：
* 文件摘要：实现简单的环形内存队列，专门用于中转（生产-消费 关系） H264 编码的视频帧。
* 
* 当前版本：1.0.0.0
* 作    者：
* 完成日期：2014年7月25日
* 版本摘要：
* 
* 取代版本：
* 原作者  ：
* 完成日期：
* 版本摘要：
* 
*/

#include <list>
#include <Windows.h>

///////////////////////////////////////////////////////////////////////////
// x_lock_t

class x_lock_t
{
	// constructor/destructor
public:
	x_lock_t(void)
	{
		InitializeCriticalSection(&m_cs_lock);
	}

	~x_lock_t(void)
	{
#ifdef _DEBUG
		EnterCriticalSection(&m_cs_lock);
		LeaveCriticalSection(&m_cs_lock);
#endif // _DEBUG

		DeleteCriticalSection(&m_cs_lock);
	}

	// public interfaces
public:
	inline void lock(void)
	{
		EnterCriticalSection(&m_cs_lock);
	}

	inline void unlock(void)
	{
		LeaveCriticalSection(&m_cs_lock);
	}

	// class data
protected:
	CRITICAL_SECTION   m_cs_lock;

};

class x_autolock_t
{
	// constructor/destructor
public:
	x_autolock_t(x_lock_t &xt_lock)
		: m_xt_lock(&xt_lock)
	{
		m_xt_lock->lock();
	}

	~x_autolock_t(void)
	{
		m_xt_lock->unlock();
	}

	// class data
private:
	x_lock_t   * m_xt_lock;
};

///////////////////////////////////////////////////////////////////////////
// xmemblock definition

class xmemblock
{
	// class properties
public:
	typedef enum ConstValueID
	{
		BLOCK_INIT_SIZE = 4 * 1024,
	} ConstValueID;

	// constructor/destructor
public:
	xmemblock(size_t st_size = BLOCK_INIT_SIZE);
	~xmemblock(void);

	// public interfaces
public:
	/**************************************************************************
	* Description:
	*     将数据附加到内存块尾部。
	* Parameter:
	*     @[in ] src_buf: 指向源数据内存区域。
	*     @[in ] st_size: 源数据内存区域大小（按字节计）。
	* ReturnValue:
	*     返回实际写入数据量（按字节计）；
	*     返回 -1，表示产生错误。
	*/
	int append_data(const void * src_buf, size_t st_size);

	/**************************************************************************
	* Description:
	*     将数据写入内存块。
	* Parameter:
	*     @[in ] src_buf: 指向源数据内存区域。
	*     @[in ] st_size: 源数据内存区域大小（按字节计）。
	* ReturnValue:
	*     返回实际写入数据量（按字节计）；
	*     返回 -1，表示产生错误。
	*/
	int write_block(const void * src_buf, size_t st_size);

	/**************************************************************************
	* Description:
	*     将内存块数据读取至目标内存区域。
	* Parameter:
	*     @[out] dst_buf: 目标内存区域。
	*     @[in ] st_size: 目标内存区域大小。
	* ReturnValue:
	*     返回实际读取数据量（按字节计）；
	*     返回 -1，表示产生错误。
	*/
	int read_block(void *dst_buf, size_t st_size);

	/**************************************************************************
	* Description:
	*     设置内存块的空间大小（最大读写空间大小，注意，该操作将丢弃原内存块中的数据）。
	* Parameter:
	*     @[in ] st_max_size: 内存块的空间大小。
	* ReturnValue:
	*     返回新分配的内存块空间大小；
	*     返回 -1，表示产生错误。
	*/
	int set_max_size(size_t st_max_size);

	/**************************************************************************
	* Description:
	*     自动增长内存块的空间大小值；
	*     若增长值未超过内存块空间的最大值，则不重新分配内存区；
	*     若增长值超过内存块空间的最大值，则调用 set_max_size() 分配内存值。
	* Parameter:
	*     @[in ] st_auto_resize: 自动增长值。
	* ReturnValue:
	*     返回内存块的空间大小（最大读写空间）。
	*/
	int auto_resize(size_t st_auto_resize);

	/**************************************************************************
	* Description:
	*     返回指向内存块的数据地址。
	*/
	inline const void * data(void)
	{
		return m_data_ptr;
	}

	/**************************************************************************
	* Description:
	*     返回内存块当前使用到的数据区域大小的引用（与数据块起始位置的偏移量）。
	*/
	inline size_t& size(void)
	{
		return m_st_len;
	}

	/**************************************************************************
	* Description:
	*     将写入数据的长度重置零。
	*/
	inline void reset(void)
	{
		m_st_len = 0;
	}

	/**************************************************************************
	* Description:
	*     返回内存块的空间大小（最大读写空间）。
	*/
	inline size_t max_size(void) const
	{
		return m_st_max;
	}

	// class data
protected:
	unsigned char * m_data_ptr;
	size_t          m_st_max;
	size_t          m_st_len;
};

///////////////////////////////////////////////////////////////////////////
// xmemblock implementation

//=========================================================================

// 
// xmemblock constructor/destructor
// 

xmemblock::xmemblock(size_t st_size /* = BLOCK_INIT_SIZE */)
{
	m_data_ptr = (unsigned char *)calloc(st_size, sizeof(char));
	m_st_len = 0;
	m_st_max = st_size;
}

xmemblock::~xmemblock(void)
{
	if (NULL != m_data_ptr)
	{
		free(m_data_ptr);
	}
}

//=========================================================================

// 
// xmemblock public interfaces
// 

/**************************************************************************
* Description:
*     将数据附加到内存块尾部。
* Parameter:
*     @[in ] src_buf: 指向源数据内存区域。
*     @[in ] st_size: 源数据内存区域大小（按字节计）。
* ReturnValue:
*      返回实际写入数据量（按字节计）；
*      返回 -1，表示产生错误。
*/
int xmemblock::append_data(const void *src_buf, size_t st_size)
{
	if ((NULL == src_buf) || (0 == st_size))
		return 0;

	size_t new_len = st_size + size();

	if (new_len > m_st_max)
	{
		unsigned char * _data_new_ptr = (unsigned char *)((NULL != m_data_ptr) ?
			realloc(m_data_ptr, new_len) : calloc(new_len, sizeof(char)));

		if (NULL == _data_new_ptr)
			return -1;

		m_data_ptr = _data_new_ptr;
		m_st_max = new_len;
	}

	memcpy(m_data_ptr + size(), src_buf, st_size);
	m_st_len = new_len;

	return (int)m_st_len;
}

/**************************************************************************
* Description:
*     将数据写入内存块。
* Parameter:
*     @[in ] src_buf: 指向源数据内存区域。
*     @[in ] st_size: 源数据内存区域大小（按字节计）。
* ReturnValue:
*      返回实际写入数据量（按字节计）；
*      返回 -1，表示产生错误。
*/
int xmemblock::write_block(const void *src_buf, size_t st_size)
{
	if ((NULL == src_buf) || (0 == st_size))
		return 0;

	if (st_size > m_st_max)
	{
		unsigned char * _data_new_ptr = (unsigned char *)malloc(st_size);
		if (NULL == _data_new_ptr)
			return -1;

		if (NULL != m_data_ptr)
			free(m_data_ptr);
		m_data_ptr = _data_new_ptr;
		m_st_max = st_size;
	}

	memcpy(m_data_ptr, src_buf, st_size);
	m_st_len = st_size;

	return (int)m_st_len;
}

/**************************************************************************
* Description:
*     将内存块数据读取至目标内存区域。
* Parameter:
*     @[out] dst_buf: 目标内存区域。
*     @[in ] st_size: 目标内存区域大小。
* ReturnValue:
*      返回实际读取数据量（按字节计）；
*      返回 -1，表示产生错误。
*/
int xmemblock::read_block(void *dst_buf, size_t st_size)
{
	if ((NULL == dst_buf) || (0 == st_size))
		return 0;

	int read_bytes = (st_size < m_st_len) ? st_size : m_st_len;
	memcpy(dst_buf, m_data_ptr, read_bytes);

	return read_bytes;
}

/**************************************************************************
* Description:
*     设置内存块的空间大小（最大读写空间大小，注意，该操作将丢弃原内存块中的数据）。
* Parameter:
*     @[in] st_max_size: 内存块的空间大小。
* ReturnValue:
*      返回新分配的内存块空间大小；
*      返回 -1，表示产生错误。
*/
int xmemblock::set_max_size(size_t st_max_size)
{
	unsigned char * _data_new_ptr = (unsigned char *)malloc(st_max_size);
	if (NULL == _data_new_ptr)
		return -1;

	if (NULL != m_data_ptr)
		free(m_data_ptr);
	m_data_ptr = _data_new_ptr;
	m_st_max = st_max_size;

	return m_st_max;
}

/**************************************************************************
* Description:
*     自动增长内存块的空间大小值；
*     若增长值未超过内存块空间的最大值，则不重新分配内存区；
*     若增长值超过内存块空间的最大值，则调用 set_max_size() 分配内存值。
* Parameter:
*     @[in ] st_auto_resize: 自动增长值。
* ReturnValue:
*     返回内存块的空间大小（最大读写空间）。
*/
int xmemblock::auto_resize(size_t st_auto_resize)
{
	if (st_auto_resize > max_size())
	{
		set_max_size(st_auto_resize);
	}
	return max_size();
}

///////////////////////////////////////////////////////////////////////////
// xmemblock_cirqueue definition

class xmemblock_cirqueue
{
	// class property
public:
	typedef enum ConstValueID
	{
		QUEUE_INIT_MAX_BLOCKS = 500,
	} ConstValueID;

	// constructor/destructor
public:
	xmemblock_cirqueue(size_t st_max_blocks = QUEUE_INIT_MAX_BLOCKS);
	~xmemblock_cirqueue(void);

	// public interfaces
public:
	/**************************************************************************
	* Description:
	*     申请内存块操作对象。
	*     注意:申请对象操作，首先判断空闲队列中是否为空，若不为空，则取空闲队列中的对象
	*          作为申请对象返回，若为空，则判断存储队列是否达到最大存储数量，若达到，
	*          则取存储队列顶端对象作为申请对象返回，否则新创建对象返回。
	* Parameter:
	*     
	* ReturnValue:
	*     成功，返回申请的内存块对象；
	*     失败，返回 NULL。
	*/
	xmemblock * alloc(void);

	/**************************************************************************
	* Description:
	*     回收内存块对象。
	* Parameter:
	*     @[in ] _block_ptr: 要回收的内存块对象。
	* ReturnValue:
	*     void
	*/
	void recyc(xmemblock *_block_ptr);

	/**************************************************************************
	* Description:
	*     将需要保存数据的内存块对象压入环形内存块队列的存储队列底端。
	* Parameter:
	*     @[in ] _block_ptr: 要保存数据的内存块对象。
	* ReturnValue:
	*     void
	*/
	void push_back_to_saved_queue(xmemblock *_block_ptr);

	/**************************************************************************
	* Description:
	*     从存储队列中取出顶端的内存块对象，然后通过该对象操作相应的数据。
	* ReturnValue:
	*     返回最早被写入数据的内存块对象；
	*     若返回 NULL ，表示存储队列为空。
	*/
	xmemblock * pop_front_from_saved_queue(void);

	/**************************************************************************
	* FunctionName:
	*     cleanup_saved_queue
	* Description:
	*     清理掉所有已保存的内存块对象（回收至空闲队列中）。
	*/
	void cleanup_saved_queue(void);

	/**************************************************************************
	* Description:
	*     将数据写入存储内存块队列中。
	* Parameter:
	*     @[in ] src_buf: 源数据。
	*     @[in ] st_len: 源数据长度。
	* ReturnValue:
	*     实际写入数据的长度（按字节计）；
	*     返回 -1，表示产生错误。
	*/
	int write_in(const void * src_buf, size_t st_len);

	/**************************************************************************
	* Description:
	*     从存储队列中取出顶端的内存块对象的数据。
	* Parameter:
	*     @[out] dst_buf: 接收数据的目标内存缓冲区。
	*     @[in ] st_len: 目标内存缓冲区大小。
	* ReturnValue:
	*     实际读取数据的长度（按字节计）；
	*     返回 -1，表示产生错误。
	*/
	int read_out(void * dst_buf, size_t st_len);

	/**************************************************************************
	* Description:
	*     返回环形内存块队列的最大管理内存块对象数量。
	*/
	inline size_t max_blocks_size(void) const
	{
		return m_st_max_blocks;
	}

	/**************************************************************************
	* Description:
	*     设置环形内存块队列的最大管理内存块对象数量。
	* Parameter:
	*     @[in ] st_max_blocks: 最大管理内存块对象数量。
	* ReturnValue:
	*     返回设置前 最大管理内存块对象数量。
	*/
	size_t resize_max_blocks(size_t st_max_blocks);

	/**************************************************************************
	* Description:
	*     释放环形内存块队列中所有的内存块对象。
	*/
	void clear_cir_queue(void);

	// class data
protected:
	x_lock_t                   m_lock_lst_free;     // 空闲的内存块对象的队列访问锁
	std::list< xmemblock * >   m_lst_free;          // 空闲的内存块对象
	x_lock_t                   m_lock_lst_save;     // 存储的内存块对象的队列访问锁
	std::list< xmemblock * >   m_lst_save;          // 存储的内存块对象
	volatile size_t            m_st_max_blocks;     // 最大管理内存块对象数量

};

///////////////////////////////////////////////////////////////////////////
// 

//=========================================================================

// 
// constructor/destructor
// 

xmemblock_cirqueue::xmemblock_cirqueue(size_t st_max_blocks /* = PAGE_INIT_MAX_BLOCKS */)
		: m_st_max_blocks(st_max_blocks)
{

}

xmemblock_cirqueue::~xmemblock_cirqueue(void)
{
	clear_cir_queue();
}

//=========================================================================

// 
// public interfaces
// 

/**************************************************************************
* Description:
*     申请内存块操作对象。
*     注意:申请对象操作，首先判断空闲队列中是否为空，若不为空，则取空闲队列中的对象
*          作为申请对象返回，若为空，则判断存储队列是否达到最大存储数量，若达到，
*          则取存储队列顶端对象作为申请对象返回，否则新创建对象返回。
* Parameter:
*     
* ReturnValue:
*      成功，返回申请的内存块对象；
*      失败，返回 NULL。
*/
xmemblock * xmemblock_cirqueue::alloc(void)
{
	xmemblock * _block_ptr = NULL;

	x_autolock_t auto_lock_free(m_lock_lst_free);
	if (m_lst_free.empty())
	{
		x_autolock_t auto_lock_save(m_lock_lst_save);
		if (m_lst_save.size() < max_blocks_size())
			_block_ptr = new xmemblock((size_t)xmemblock::BLOCK_INIT_SIZE);
		else
		{
			_block_ptr = m_lst_save.front();
			m_lst_save.pop_front();
		}
	}
	else
	{
		_block_ptr = m_lst_free.front();
		m_lst_free.pop_front();
	}

	return _block_ptr;
}

/**************************************************************************
* Description:
*     回收内存块对象。
* Parameter:
*     @[in ] _block_ptr: 要回收的内存块对象。
* ReturnValue:
*     void
*/
void xmemblock_cirqueue::recyc(xmemblock * _block_ptr)
{
	if (NULL == _block_ptr)
		return;

	x_autolock_t auto_lock_free(m_lock_lst_free);
	if (m_lst_free.size() < max_blocks_size())
		m_lst_free.push_back(_block_ptr);
	else
		delete _block_ptr;
}

/**************************************************************************
* Description:
*     将需要保存数据的内存块对象压入环形内存块队列的存储队列底端。
* Parameter:
*     @[in ] _block_ptr: 要保存数据的内存块对象。
* ReturnValue:
*     void
*/
void xmemblock_cirqueue::push_back_to_saved_queue(xmemblock *_block_ptr)
{
	if (NULL == _block_ptr)
		return;

	x_autolock_t auto_lock_save(m_lock_lst_save);
	m_lst_save.push_back(_block_ptr);
}

/**************************************************************************
* Description:
*     从存储队列中取出顶端的内存块对象，然后通过该对象操作相应的数据。
* ReturnValue:
*     返回最早被写入数据的内存块对象；
*     若返回 NULL ，表示存储队列为空。
*/
xmemblock * xmemblock_cirqueue::pop_front_from_saved_queue(void)
{
	x_autolock_t auto_lock_save(m_lock_lst_save);

	xmemblock *_block_ptr = NULL;
	if (!m_lst_save.empty())
	{
		_block_ptr = m_lst_save.front();
		m_lst_save.pop_front();
	}

	return _block_ptr;
}

/**************************************************************************
* FunctionName:
*     cleanup_saved_queue
* Description:
*     清理掉所有已保存的内存块对象（回收至空闲队列中）。
*/
void xmemblock_cirqueue::cleanup_saved_queue(void)
{
	x_autolock_t auto_lock_save(m_lock_lst_save);
	x_autolock_t auto_lock_free(m_lock_lst_free);

	std::list< xmemblock * >::iterator itlst = m_lst_save.begin();
	for (; itlst != m_lst_save.end(); ++itlst)
	{
		xmemblock * _block_ptr = *itlst;
		if (m_lst_free.size() < max_blocks_size())
			m_lst_free.push_back(_block_ptr);
		else
			delete _block_ptr;
	}

	m_lst_save.clear();
}

/**************************************************************************
* Description:
*     将数据写入存储内存块队列中。
* Parameter:
*     @[in ] src_buf: 源数据。
*     @[in ] st_len: 源数据长度。
* ReturnValue:
*     实际写入数据的长度（按字节计）；
*     返回 -1，表示产生错误。
*/
int xmemblock_cirqueue::write_in(const void * src_buf, size_t st_len)
{
	if ((NULL == src_buf) || (0 == st_len))
		return 0;

	xmemblock * _block_ptr = alloc();
	if (NULL == _block_ptr)
		return 0;

	int i_write = _block_ptr->write_block(src_buf, st_len);
	if (i_write > 0)
		push_back_to_saved_queue(_block_ptr);
	else
		recyc(_block_ptr);

	return i_write;
}

/**************************************************************************
* Description:
*     从存储队列中取出顶端的内存块对象的数据。
* Parameter:
*     @[out] dst_buf: 接收数据的目标内存缓冲区。
*     @[in ] st_len: 目标内存缓冲区大小。
* ReturnValue:
*     实际读取数据的长度（按字节计）；
*     返回 -1，表示产生错误。
*/
int xmemblock_cirqueue::read_out(void * dst_buf, size_t st_len)
{
	if ((NULL == dst_buf) || (0 == st_len))
		return 0;

	xmemblock * _block_ptr = pop_front_from_saved_queue();
	if ((NULL == _block_ptr) || (0 == _block_ptr->size()))
		return 0;
	int i_read = _block_ptr->read_block(dst_buf, st_len);
	recyc(_block_ptr);

	return i_read;
}

/**************************************************************************
* Description:
*     设置环形内存块队列的最大管理内存块对象数量。
* Parameter:
*     @[in ] st_max_blocks: 最大管理内存块对象数量。
* ReturnValue:
*     返回设置前 最大管理内存块对象数量。
*/
size_t xmemblock_cirqueue::resize_max_blocks(size_t st_max_blocks)
{
	x_autolock_t auto_lock(m_lock_lst_free);
	while (m_lst_free.size() > st_max_blocks)
	{
		xmemblock * _block_ptr = m_lst_free.back();
		m_lst_free.pop_back();
		delete _block_ptr;
	}

	return (size_t)InterlockedExchange((LONG *)&m_st_max_blocks, (LONG)st_max_blocks);
}

/**************************************************************************
* Description:
*     释放环形内存块队列中所有的内存块对象。
*/
void xmemblock_cirqueue::clear_cir_queue(void)
{
	x_autolock_t auto_lock_free(m_lock_lst_free);
	std::list< xmemblock * >::iterator itls_free = m_lst_free.begin();
	for (; itls_free != m_lst_free.end(); ++itls_free)
	{
		delete *itls_free;
	}
	m_lst_free.clear();

	x_autolock_t auto_lock_save(m_lock_lst_save);
	std::list< xmemblock * >::iterator itls_used = m_lst_save.begin();
	for (; itls_used != m_lst_save.end(); ++itls_used)
	{
		delete *itls_used;
	}
	m_lst_save.clear();
}

