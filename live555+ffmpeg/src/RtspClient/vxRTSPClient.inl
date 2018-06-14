/*
* Copyright (c) 2014, 百年千岁
* All rights reserved.
* 
* 文件名称：vxRTSPClient.inl
* 创建日期：2014年7月25日
* 文件标识：
* 文件摘要：利用 Live555 提供的操作类，实现 RTSP 协议的客户端接收 H264 视频码流功能。
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

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "H264VideoRTPSource.hh"

///////////////////////////////////////////////////////////////////////////

#define RTSP_FRAMETYPE_RCDPARAM     101
#define RTSP_FRAMETYPE_REALH264     102

typedef void (* RTSP_REALFRAME_CBK)(void * pv_handle, void * frame_buf, unsigned int frame_size, void * pv_user);
typedef void (* CLI_REALFRAME_CBK)(void * pv_handle, void * frame_buf, unsigned int frame_size, unsigned int frame_type, void * pv_user);

///////////////////////////////////////////////////////////////////////////
// vxMediaSink definition

class vxMediaSink : public MediaSink
{
	// constructor/destructor
protected:
	vxMediaSink(UsageEnvironment &env, MediaSubsession * subsession, unsigned int recvbuf_size)
		: MediaSink(env)
		, m_subsession(subsession)
		, m_recvbuf(NULL)
		, m_recvbuf_size(0)
		, m_func_recved(NULL)
		, m_pv_user(NULL)
	{
		m_recvbuf = (unsigned char *)malloc(recvbuf_size);
		if (NULL != m_recvbuf)
		{
			m_recvbuf_size = recvbuf_size;
		}
	}

	virtual ~vxMediaSink(void)
	{
		if (NULL != m_recvbuf)
		{
			free(m_recvbuf);
			m_recvbuf = NULL;
		}
	}

	// class properties
public:
	typedef enum ConstValueID
	{
		ECV_DEF_RECVBUF_SIZE     = 1024 * 1024,
	} ConstValueID;

	// static invoking methods
public:
	/**************************************************************************
	* FunctionName:
	*     create
	* Description:
	*     Create vxMediaSink object.
	*/
	static vxMediaSink * create(UsageEnvironment &env, MediaSubsession * subsession, unsigned int recvbuf_size = ECV_DEF_RECVBUF_SIZE)
	{
		vxMediaSink * obj_ptr = new vxMediaSink(env, subsession, recvbuf_size);
		return obj_ptr;
	}

protected:
	/**************************************************************************
	* FunctionName:
	*     getting_frame_cbk
	* Description:
	*    This methods will get called later, when a frame arrives.
	*/
	static void getting_frame_cbk(void * pv_user, unsigned int frame_size,
					unsigned int truncated_bytes, struct timeval tmval_info, unsigned time_dutation)
	{
		vxMediaSink *this_sink = (vxMediaSink *)pv_user;
		this_sink->getting_frame_proc(frame_size, truncated_bytes, tmval_info, time_dutation);
	}

	// overrides
protected:
	virtual Boolean continuePlaying(void)
	{
		if (NULL == fSource)
			return False;

		// Request the next frame of data from our input source.  "m_func_recved" will get called later, when it arrives:
		fSource->getNextFrame(m_recvbuf, m_recvbuf_size,
							getting_frame_cbk, this,
							onSourceClosure, this);

		return True;
	}

	// public interfaces
public:
	/**************************************************************************
	* FunctionName:
	*     set_recved_realframe_cbk
	* Description:
	*     Setting recved callback function parameter.
	*/
	inline void set_recved_realframe_cbk(RTSP_REALFRAME_CBK func_recved, void * pv_user)
	{
		m_func_recved = func_recved;
		m_pv_user     = pv_user;
	}

	/**************************************************************************
	* FunctionName:
	*     recvbuf
	* Description:
	*     return recv buffer address.
	*/
	inline unsigned char * recvbuf(void) const { return m_recvbuf; }

	/**************************************************************************
	* FunctionName:
	*     subsession
	* Description:
	*     return object belongs to the subsession.
	*/
	inline MediaSubsession * subsession(void) const { return m_subsession; }

	// inner invoking methods
protected:
	/**************************************************************************
	* FunctionName:
	*     getting_frame_proc
	* Description:
	*     Processing recved the frame.
	*/
	void getting_frame_proc(unsigned int frame_size, unsigned int truncated_bytes,
							struct timeval tmval_info, unsigned time_dutation)
	{
		if (NULL != m_func_recved)
		{
			m_func_recved(this, m_recvbuf, frame_size, m_pv_user);
		}

		// Then continue, to request the next frame of data:
		continuePlaying();
	}

	// class data
protected:
	MediaSubsession    * m_subsession;
	unsigned char      * m_recvbuf;
	unsigned int         m_recvbuf_size;
	RTSP_REALFRAME_CBK   m_func_recved;
	void               * m_pv_user;

};

///////////////////////////////////////////////////////////////////////////
// vxRtspCliHandle definition

class vxRtspCliHandle : public RTSPClient
{
	// constructor/destructor
protected:
	vxRtspCliHandle(UsageEnvironment * env, const char * sz_rtsp_url);
	virtual ~vxRtspCliHandle(void);

	// static invoking methods
public:
	/**************************************************************************
	* FunctionName:
	*     create
	* Description:
	*     Create vxRtspCliHandle object.
	*/
	static vxRtspCliHandle * create(const char * sz_rtsp_url);

	/**************************************************************************
	* FunctionName:
	*     destroy
	* Description:
	*     Destroy vxRtspCliHandle object.
	*/
	static void destroy(vxRtspCliHandle * obj_ptr);

	//=========================================================================
	// RTSP response handlers

protected:
	/**************************************************************************
	* FunctionName:
	*     RtspClient_OnResponse_DESCRIBE
	* Description:
	*     Processing DESCRIBE response message.
	*/
	static void RtspClient_OnResponse_DESCRIBE(RTSPClient * rtspClient, int resultCode, char * resultString);

	/**************************************************************************
	* FunctionName:
	*     RtspClient_OnResponse_SETUP
	* Description:
	*     Processing SETUP response message.
	*/
	static void RtspClient_OnResponse_SETUP(RTSPClient * rtspClient, int resultCode, char * resultString);

	/**************************************************************************
	* FunctionName:
	*     RtspClient_OnResponse_PLAY
	* Description:
	*     Processing PLAY response message.
	*/
	static void RtspClient_OnResponse_PLAY(RTSPClient *rtspClient, int resultCode, char *resultString);

	/**************************************************************************
	* FunctionName:
	*     Subsession_OnEvent_Playing
	* Description:
	*     called when a stream's subsession (e.g., audio or video substream) ends.
	*/
	static void Subsession_OnEvent_Playing(void * clientData);

	/**************************************************************************
	* FunctionName:
	*     Subsession_OnEvent_ByeHandler
	* Description:
	*     called when a RTCP "BYE" is received for a subsession.
	*/
	static void Subsession_OnEvent_ByeHandler(void * clientData);

	/**************************************************************************
	* FunctionName:
	*     Stream_OnEvent_TimerHandler
	* Description:
	*     called at the end of a stream's expected duration;
	*     if the stream has not already signaled its end using a RTCP "BYE".
	*/
	static void Stream_OnEvent_TimerHandler(void * clientData);

	/**************************************************************************
	* FunctionName:
	*     Stream_OnEvent_RealCallBack
	* Description:
	*     called at the vxMediaSink when it recved a real frame.
	*/
	static void Stream_OnEvent_RecvedRealFrame(void * pv_handle, void * frame_buf, unsigned int frame_size, void * pv_user);

	// public interfaces
public:
	/**************************************************************************
	* FunctionName:
	*     setup_next_subsession
	* Description:
	*     Used to iterate through each stream's 'subsessions', setting up each one.
	*/
	void setup_next_subsession(void);

	/**************************************************************************
	* FunctionName:
	*     shutdown_stream
	* Description:
	*     shutdown the rtsp stream.
	*/
	void shutdown_stream(void);

	/**************************************************************************
	* FunctionName:
	*     do_event_loop
	* Description:
	*     Used to processing event handler loop.
	* Parameter:
	*     @[int] loop_exit_flag: exit flag.
	* ReturnValue:
	*     void
	*/
	inline void do_event_loop(char * loop_exit_flag)
	{
		m_loop_exit_flag = loop_exit_flag;
		if (NULL != m_usage_environment)
		{
			m_usage_environment->taskScheduler().doEventLoop(m_loop_exit_flag);
		}
	}

	/**************************************************************************
	* FunctionName:
	*     set_recved_realframe_cbk
	* Description:
	*     Setting recved callback function parameter.
	*/
	inline void set_recved_realframe_cbk(CLI_REALFRAME_CBK func_recved, void * pv_user)
	{
		m_func_recved = func_recved;
		m_pv_user     = pv_user;
	}

	// class data
protected:
	TaskScheduler              * m_task_scheduler;
	UsageEnvironment           * m_usage_environment;
	char                       * m_loop_exit_flag;

	double                       m_duration;
	TaskToken                    m_stream_timer_task;
	MediaSubsessionIterator    * m_mss_iter;
	MediaSession               * m_session;
	MediaSubsession            * m_subsession;

	CLI_REALFRAME_CBK            m_func_recved;
	void                       * m_pv_user;

};

///////////////////////////////////////////////////////////////////////////
// vxRtspCliHandle implementation

//=========================================================================

// 
// vxRtspCliHandle static invoking methods
// 

/**************************************************************************
* FunctionName:
*     create
* Description:
*     Create vxRtspCliHandle object.
*/
vxRtspCliHandle * vxRtspCliHandle::create(const char * sz_rtsp_url)
{
	vxRtspCliHandle * obj_ptr = NULL;

	do 
	{
		if (NULL == sz_rtsp_url)
		{
			break;
		}

		// Begin by setting up our usage environment:
		TaskScheduler * task_scheduler_ptr = BasicTaskScheduler::createNew();
		if (NULL == task_scheduler_ptr)
		{
			break;
		}

		UsageEnvironment * usage_environment_ptr = BasicUsageEnvironment::createNew(*task_scheduler_ptr);
		if (NULL == usage_environment_ptr)
		{
			delete task_scheduler_ptr;
			break;
		}

		// create rtsp client object
		obj_ptr = new vxRtspCliHandle(usage_environment_ptr, sz_rtsp_url);
		if (NULL == obj_ptr)
		{
			usage_environment_ptr->reclaim();
			delete task_scheduler_ptr;
			break;
		}
		obj_ptr->m_task_scheduler = task_scheduler_ptr;

		// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
		// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
		// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
		obj_ptr->sendDescribeCommand(&vxRtspCliHandle::RtspClient_OnResponse_DESCRIBE);
		
	} while (0);

	return obj_ptr;
}

/**************************************************************************
* FunctionName:
*     destroy
* Description:
*     Destroy vxRtspCliHandle object.
*/
void vxRtspCliHandle::destroy(vxRtspCliHandle * obj_ptr)
{
	UsageEnvironment * usage_environment = obj_ptr->m_usage_environment;
	obj_ptr->m_usage_environment = NULL;

	TaskScheduler * task_scheduler = obj_ptr->m_task_scheduler;
	obj_ptr->m_task_scheduler = NULL;

	Medium::close(obj_ptr);

	if (NULL != usage_environment)
	{
		usage_environment->reclaim();
		usage_environment = NULL;
	}

	if (NULL != task_scheduler)
	{
		delete task_scheduler;
		task_scheduler = NULL;
	}
}

// 
// vxRtspCliHandle RTSP response handlers
// 

/**************************************************************************
* FunctionName:
*     RtspClient_OnResponse_DESCRIBE
* Description:
*     Processing DESCRIBE response message.
*/
void vxRtspCliHandle::RtspClient_OnResponse_DESCRIBE(RTSPClient * rtspClient, int resultCode, char * resultString)
{
	vxRtspCliHandle * obj_ptr = (vxRtspCliHandle *)rtspClient;

	do
	{
		UsageEnvironment &env = obj_ptr->envir();

		if (0 != resultCode)
		{
			env << obj_ptr->url() << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
			break;
		}

		const char * sdpDescription = resultString;
		env << obj_ptr->url() << "Got a SDP description:\n" << sdpDescription << "\n";

		// Create a media session object from this SDP description:
		obj_ptr->m_session = MediaSession::createNew(env, sdpDescription);
		delete[] sdpDescription;

		if (NULL == obj_ptr->m_session)
		{
			env << obj_ptr->url() << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			break;
		}
		else if (!obj_ptr->m_session->hasSubsessions())
		{
			env << obj_ptr->url() << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			break;
		}

		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		obj_ptr->m_mss_iter = new MediaSubsessionIterator(*obj_ptr->m_session);
		obj_ptr->setup_next_subsession();

		return ;

	} while (0);

	// An unrecoverable error occurred with this stream.
	obj_ptr->shutdown_stream();
}

/**************************************************************************
* FunctionName:
*     RtspClient_OnResponse_SETUP
* Description:
*     Processing SETUP response message.
*/
void vxRtspCliHandle::RtspClient_OnResponse_SETUP(RTSPClient * rtspClient, int resultCode, char * resultString)
{
	vxRtspCliHandle * obj_ptr = (vxRtspCliHandle *)rtspClient;

	do
	{
		UsageEnvironment &env = obj_ptr->envir();

		if (0 != resultCode)
		{
			env << obj_ptr->url() << "Failed to set up the \""
				<< obj_ptr->m_subsession->mediumName() << "/"
				<< obj_ptr->m_subsession->codecName() << "\" subsession: " << resultString << "\n";
			break;
		}

#ifdef _DEBUG

		env << obj_ptr->url() << "Set up the \""
			<< obj_ptr->m_subsession->mediumName() << "/"
			<< obj_ptr->m_subsession->codecName() << "\" subsession (";

		if (obj_ptr->m_subsession->rtcpIsMuxed())
			env << "client port " << obj_ptr->m_subsession->clientPortNum();
		else
			env << "client ports " << obj_ptr->m_subsession->clientPortNum() << "-" << obj_ptr->m_subsession->clientPortNum() + 1;

		env << ")\n";

#endif

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)

		vxMediaSink * sink_ptr = vxMediaSink::create(env, obj_ptr->m_subsession, vxMediaSink::ECV_DEF_RECVBUF_SIZE);

		// perhaps use your own custom "MediaSink" subclass instead
		if (NULL == sink_ptr)
		{
			env << obj_ptr->url() << "Failed to create a data sink for the \""
				<< obj_ptr->m_subsession->mediumName() << "/"
				<< obj_ptr->m_subsession->codecName()
				<< "\" subsession: " << env.getResultMsg() << "\n";
			break;
		}

		env << obj_ptr->url() << "Created a data sink for the \""
			<< obj_ptr->m_subsession->mediumName() << "/"
			<< obj_ptr->m_subsession->codecName() << "\" subsession\n";

		// a hack to let subsession handle functions get the "RTSPClient" from the subsession
		sink_ptr->set_recved_realframe_cbk(&vxRtspCliHandle::Stream_OnEvent_RecvedRealFrame, obj_ptr);
		obj_ptr->m_subsession->sink = sink_ptr;
		obj_ptr->m_subsession->miscPtr = obj_ptr;
		obj_ptr->m_subsession->sink->startPlaying(*(obj_ptr->m_subsession->readSource()),
									&vxRtspCliHandle::Subsession_OnEvent_Playing, obj_ptr->m_subsession);

		// Get SPS and PPS data for decode H264 video stream.
		unsigned int nSPropRecords = 0;
		SPropRecord * pSPropRecord = parseSPropParameterSets(obj_ptr->m_subsession->fmtp_spropparametersets(), nSPropRecords);

		if ((NULL != obj_ptr->m_func_recved) && (nSPropRecords > 0))
		{
			for (unsigned int i = 0; i < nSPropRecords; ++i)
			{
				SPropRecord * sps_ptr = &pSPropRecord[i];
				obj_ptr->m_func_recved(obj_ptr, pSPropRecord[i].sPropBytes, pSPropRecord[i].sPropLength, RTSP_FRAMETYPE_RCDPARAM, obj_ptr->m_pv_user);
			}

			obj_ptr->m_func_recved(obj_ptr, NULL, 0, RTSP_FRAMETYPE_RCDPARAM, obj_ptr->m_pv_user);

			delete[] pSPropRecord;
		}

		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (NULL != obj_ptr->m_subsession->rtcpInstance())
		{
			obj_ptr->m_subsession->rtcpInstance()->setByeHandler(&vxRtspCliHandle::Subsession_OnEvent_ByeHandler, obj_ptr->m_subsession);
		}

	} while (0);

	delete[] resultString;

	// Set up the next subsession, if any:
	obj_ptr->setup_next_subsession();
}

/**************************************************************************
* FunctionName:
*     RtspClient_OnResponse_PLAY
* Description:
*     Processing PLAY response message.
*/
void vxRtspCliHandle::RtspClient_OnResponse_PLAY(RTSPClient *rtspClient, int resultCode, char *resultString)
{
	vxRtspCliHandle * obj_ptr = (vxRtspCliHandle *)rtspClient;

	Boolean b_ok = False;

	do
	{
		UsageEnvironment &env = obj_ptr->envir();

		if (resultCode != 0)
		{
			env << obj_ptr->url() << "Failed to start playing session: " << resultString << "\n";
			break;
		}

		// Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
		// using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
		// 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
		// (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
		if (obj_ptr->m_duration > 0)
		{
			// number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
			const unsigned int delaySlop = 2;
			obj_ptr->m_duration += delaySlop;

			unsigned int uSecsToDelay = (unsigned int)(obj_ptr->m_duration * 1000000);
			obj_ptr->m_stream_timer_task =
				env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc *)&vxRtspCliHandle::Stream_OnEvent_TimerHandler, obj_ptr);
		}

		env << obj_ptr->url() << "Started playing session";
		if (obj_ptr->m_duration > 0)
		{
			env << " (for up to " << obj_ptr->m_duration << " seconds)";
		}

		env << "...\n";

		b_ok = True;
	}
	while (0);

	delete[] resultString;

	if (!b_ok)
	{
		// An unrecoverable error occurred with this stream.
		obj_ptr->shutdown_stream();
	}
}

/**************************************************************************
* FunctionName:
*     Subsession_OnEvent_Playing
* Description:
*     called when a stream's subsession (e.g., audio or video substream) ends.
*/
void vxRtspCliHandle::Subsession_OnEvent_Playing(void * clientData)
{
	MediaSubsession *subsession = (MediaSubsession *)clientData;
	vxRtspCliHandle *obj_ptr    = (vxRtspCliHandle *)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession &session = subsession->parentSession();
	MediaSubsessionIterator iter(session);

	while (NULL != (subsession = iter.next()))
	{
		// this subsession is still active
		if (subsession->sink != NULL)
			return;
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	obj_ptr->shutdown_stream();
}

/**************************************************************************
* FunctionName:
*     Subsession_OnEvent_ByeHandler
* Description:
*     called when a RTCP "BYE" is received for a subsession.
*/
void vxRtspCliHandle::Subsession_OnEvent_ByeHandler(void * clientData)
{
	MediaSubsession * subsession = (MediaSubsession *)clientData;
	vxRtspCliHandle * obj_ptr    = (vxRtspCliHandle *)subsession->miscPtr;

	UsageEnvironment &env = obj_ptr->envir();
	env << obj_ptr->url() << "Received RTCP \"BYE\" on \""
		<< subsession->mediumName() << "/" << subsession->codecName() << "\" subsession\n";

	// Now act as if the subsession had closed:
	vxRtspCliHandle::Subsession_OnEvent_Playing(subsession);
}

/**************************************************************************
* FunctionName:
*     Stream_OnEvent_TimerHandler
* Description:
*     called at the end of a stream's expected duration;
*     if the stream has not already signaled its end using a RTCP "BYE".
*/
void vxRtspCliHandle::Stream_OnEvent_TimerHandler(void * clientData)
{
	vxRtspCliHandle * obj_ptr = (vxRtspCliHandle *)clientData;
	obj_ptr->m_stream_timer_task = NULL;

	// Shut down the stream:
	obj_ptr->shutdown_stream();
}

/**************************************************************************
* FunctionName:
*     Stream_OnEvent_RealCallBack
* Description:
*     called at the vxMediaSink when it recved a real frame.
*/
void vxRtspCliHandle::Stream_OnEvent_RecvedRealFrame(void * pv_handle, void * frame_buf, unsigned int frame_size, void * pv_user)
{
	vxMediaSink     * sink_ptr = (vxMediaSink *)pv_handle;
	vxRtspCliHandle * obj_ptr  = (vxRtspCliHandle *)pv_user;

	MediaSubsession * subsession_ptr = sink_ptr->subsession();

	const char * sz_media_name = subsession_ptr->mediumName();
	const char * sz_codec_name = subsession_ptr->codecName();

	if ((0 == _stricmp(sz_media_name, "video")) && (0 == _stricmp(sz_codec_name, "H264")))
	{
		if (NULL != obj_ptr->m_func_recved)
		{
			obj_ptr->m_func_recved(obj_ptr, frame_buf, frame_size, RTSP_FRAMETYPE_REALH264, obj_ptr->m_pv_user);
		}
	}
}

//=========================================================================

// 
// vxRtspCliHandle constructor/destructor
// 

vxRtspCliHandle::vxRtspCliHandle(UsageEnvironment * env, const char * sz_rtsp_url)
			: RTSPClient(*env, sz_rtsp_url, 0, NULL, 0, -1)
			, m_task_scheduler(NULL)
			, m_usage_environment(env)
			, m_loop_exit_flag(NULL)
			, m_duration(0.0)
			, m_stream_timer_task(NULL)
			, m_mss_iter(NULL)
			, m_session(NULL)
			, m_subsession(NULL)
			, m_func_recved(NULL)
			, m_pv_user(NULL)
{

}

vxRtspCliHandle::~vxRtspCliHandle(void)
{

}

//=========================================================================

// 
// vxRtspCliHandle public interfaces
// 

/**************************************************************************
* FunctionName:
*     setup_next_subsession
* Description:
*     Used to iterate through each stream's 'subsessions', setting up each one.
*/
void vxRtspCliHandle::setup_next_subsession(void)
{
	if (NULL == m_mss_iter)
	{
		return;
	}

	m_subsession = m_mss_iter->next();

	if (NULL != m_subsession)
	{
		if (!m_subsession->initiate())
		{
			// give up on this subsession; go to the next one
			setup_next_subsession();
		}
		else
		{
			// Continue setting up this subsession, by sending a RTSP "SETUP" command.
			// By default, we request that the server stream its data using RTP/UDP.
			// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
			const Boolean request_streaming_over_tcp = False;
			sendSetupCommand(*m_subsession, &vxRtspCliHandle::RtspClient_OnResponse_SETUP, False, request_streaming_over_tcp);
		}

		return;
	}

	// We've finished setting up all of the subsessions. Now, send a RTSP "PLAY" command to start the streaming:
	if (NULL != m_session->absStartTime())
	{
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		sendPlayCommand(*m_session, &vxRtspCliHandle::RtspClient_OnResponse_PLAY, m_session->absStartTime(), m_session->absEndTime());
	}
	else
	{
		m_duration = m_session->playEndTime() - m_session->playStartTime();
		sendPlayCommand(*m_session, &vxRtspCliHandle::RtspClient_OnResponse_PLAY);
	}
}

/**************************************************************************
* FunctionName:
*     shutdown_stream
* Description:
*     shutdown the rtsp stream.
*/
void vxRtspCliHandle::shutdown_stream(void)
{
	UsageEnvironment &env = envir();

	// First, check whether any subsessions have still to be closed:
	if (NULL != m_session)
	{
		bool someSubsessionsWereActive = false;

		MediaSubsessionIterator iter(*m_session);
		MediaSubsession * subsession_ptr;
		while (NULL != (subsession_ptr = iter.next()))
		{
			if (NULL != subsession_ptr->sink)
			{
				Medium::close(subsession_ptr->sink);
				subsession_ptr->sink = NULL;

				if (subsession_ptr->rtcpInstance() != NULL)
				{
					// in case the server sends a RTCP "BYE" while handling "TEARDOWN"
					subsession_ptr->rtcpInstance()->setByeHandler(NULL, NULL);
				}

				someSubsessionsWereActive = true;
			}
		}

		if (someSubsessionsWereActive)
		{
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			sendTeardownCommand(*m_session, NULL);
		}

		env.taskScheduler().unscheduleDelayedTask(m_stream_timer_task);
		Medium::close(m_session);
		m_session = NULL;
	}

	if (NULL != m_mss_iter)
	{
		delete m_mss_iter;
		m_mss_iter = NULL;
	}

	env << url() << "Closing the stream.\n";

	// Withdraw from the event loop
	if (NULL != m_loop_exit_flag)
	{
		*m_loop_exit_flag = 1;
		m_loop_exit_flag = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////


