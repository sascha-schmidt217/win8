// (c) 2013 Sascha Schmidt

/*
#include <windows.networking.sockets.h>
#include <wrl.h>
#include <robuffer.h>
#include <vector>

using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
*/


class BBStreamSocket : public Object
{
private:

	StreamSocket^ socket;
	DataWriter^ writer;
	DataReader^ reader;
	volatile int state;
	volatile int availStatus;
	Windows::Storage::Streams::DataReaderLoadOperation^ read_op;
	Platform::Array<unsigned char>^ data;
	Microsoft::WRL::ComPtr<NativeBuffer> nativeBuffer;
	Windows::UI::Core::CoreDispatcher^ dispatcher;

protected:

	virtual void UNSAFE_ON_MESSAGE_REVEAVED(BBDataBuffer* data){} 
	virtual void UNSAFE_ON_OPENED(){} 
	virtual void UNSAFE_ON_CLOSED(){} 
	virtual void UNSAFE_ON_ERROR(){} 
	virtual void UNSAFE_ON_SEND_OK(){} 

public:

	void ON_MESSAGE_REVEAVED(BBDataBuffer* data){UNSAFE_ON_MESSAGE_REVEAVED(data);} 
	void ON_OPENED(){UNSAFE_ON_OPENED();} 
	void ON_CLOSED(){UNSAFE_ON_CLOSED();} 
	void ON_ERROR(){UNSAFE_ON_ERROR();} 
	void ON_SEND_OK(){UNSAFE_ON_SEND_OK();} 

	BBStreamSocket()
	{
		nativeBuffer = nullptr;
		state = 0;
		socket = ref new StreamSocket();
		dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;
	};

	~BBStreamSocket()
	{
		Close();
	}

	int GetDataBuffer(BBDataBuffer* dst, BBDataBuffer* src)
	{
		int* dst_tmp = ((int*)dst)+4;
		int* src_tmp = ((int*)src)+4;
		*dst_tmp = *src_tmp;
		*(dst_tmp +1)= *(src_tmp+1);
		return 0;
	}
	
	bool Connect(String addr, int port)
	{
		if( state!=0 ) return false;

		auto str_addr = ref new Platform::String(addr.ToCString<wchar_t>(), addr.Length());
		auto str_port = ref new Platform::String(String(port).ToCString<wchar_t>(), String(port).Length());

		try
		{
			auto hostName = ref new HostName(str_addr);

			auto val = socket->ConnectAsync(hostName, str_port, SocketProtectionLevel::PlainSocket);
			task<void>(val).then([this] (task<void> previousTask)  
			{
				try
				{
					previousTask.get();
					// .get() didn't throw, so we succeeded.
					state = 1;	

					OnOpen();

					// start listening
					auto reader = ref new DataReader(socket->InputStream);
					reader->InputStreamOptions = InputStreamOptions::Partial;
					reader->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;
					ReceiveLoop(reader,socket);
				}
				catch (Platform::Exception^ exception)
				{
					// "Error: failed to connect."
					state = -1;
					OnError();
				}
			});
		}
		catch (ThrowableObject* exception)
		{
			// "Error: Invalid host name."
			OnError();
			return false;
		}
		return true;
	}

	int Write( BBDataBuffer* buffer,int offset,int count ){

		if( state!=1 ) return 0;

		// write to output stream
		auto writer = ref new DataWriter(socket->OutputStream);
		writer->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;
		writer->WriteBuffer(CreateBuffer( buffer->Length(), (byte*)buffer->ReadPointer()));

		auto t = task<unsigned int>(writer->StoreAsync());
		t.then([this] (task<unsigned int> writeTask)
		{
			try
			{
				// Try getting an exception.
				writeTask.get();
				
				OnSendOk();
			}
			catch (Object* exception)
			{
				// Send failed with error: " + exception->Message
			}
		});
		return 0;
	}

	void Close()
	{
		if( socket==nullptr ) return;
		if( state==1 ) state=2;
		try
		{
			delete socket;
		}
		catch(ThrowableObject* exception)
		{
		}
		ON_CLOSED();
		socket=nullptr;
		state=-1;
	}

private:

	void OnOpen()
	{
		dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
				ref new Windows::UI::Core::DispatchedHandler([this] ()
				{
					ON_OPENED();
				}));
	}

	void OnError()
	{
		dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
				ref new Windows::UI::Core::DispatchedHandler([this] ()
				{
					ON_ERROR();
				}));
	}

	void OnMessageReceaved(BBDataBuffer* buffer)
	{
		dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
				ref new Windows::UI::Core::DispatchedHandler([this,buffer] ()
				{
					ON_MESSAGE_REVEAVED(buffer);
				}));
	}

	void OnSendOk()
	{
		dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
				ref new Windows::UI::Core::DispatchedHandler([this] ()
				{
					ON_SEND_OK();
				}));
	}

	void ReceiveLoop(DataReader^ reader, StreamSocket^ socket)
	{
		// Read first 4 bytes (length of the subsequent string).
		task<unsigned int>(reader->LoadAsync(1024)).then([this, reader, socket] (unsigned int size)
		{
			try
			{
				// get data
				BBDataBuffer* buffer= new BBDataBuffer();
				buffer->_New(size);
				auto i_buffer = reader->ReadBuffer(size);
				memcpy((uint8*) buffer->ReadPointer(), (uint8*)util::GetPointer(i_buffer), size);

				// invoke main thread event handler
				OnMessageReceaved(buffer);
			}
			catch (Platform::Exception^ exception)
			{
			}

		}).then([this, reader, socket] (task<void> previousTask)
		{
			try
			{
				// Try getting all exceptions from the continuation chain above this point.
				previousTask.get();

				// Everything went ok, so try to receive another string. 
				// The receive will continue until the stream is broken (i.e. peer closed closed the socket).
				ReceiveLoop(reader, socket);
			}
			catch (Platform::Exception^ exception)
			{
				// Explicitly close the socket.
				Close();
			}
			catch (task_canceled&)
			{
				// Do not print anything here - this will usually happen because user closed the client socket.
				// Explicitly close the socket.
				Close();
			}
		});
	}

	//
	// util
	//

	template<typename Functor>
	void Invoke(Functor& lamda)
	{
		dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
				ref new Windows::UI::Core::DispatchedHandler([this] ()
				{
					lamda();
				}));
	}
	
	IBuffer^ CreateBuffer(UINT32 cbBytes, byte* ptr)
	{
		if( nativeBuffer == nullptr )
		{
			Microsoft::WRL::MakeAndInitialize<NativeBuffer>(&nativeBuffer, cbBytes, ptr);
		}
		else
		{
			nativeBuffer->RuntimeClassInitialize(cbBytes, ptr);
		}
		auto iinspectable = (IInspectable*)reinterpret_cast<IInspectable*>(nativeBuffer.Get());
		IBuffer^ buffer = reinterpret_cast<IBuffer^>(iinspectable);
		return buffer;
	}
};
