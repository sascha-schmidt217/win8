

class BBStreamSocketListener;

//--------------------------------------------------------------

[Windows::Foundation::Metadata::WebHostHidden]
ref class BBStreamSocketListenerContext
{
private:
	BBStreamSocketListener* owner;
public:
	void SetOwner(int handle){ owner = (BBStreamSocketListener*)handle;}
	void OnConnection(StreamSocketListener^ listener, StreamSocketListenerConnectionReceivedEventArgs^ object);
};


//--------------------------------------------------------------


class BBStreamSocketListener : public Object
{
private:
	BBStreamSocketListenerContext^ context;
	StreamSocketListener^ listener;
	Windows::UI::Core::CoreDispatcher^ dispatcher;
	DataWriter^ writer;
	volatile int state;
	Microsoft::WRL::ComPtr<NativeBuffer> nativeBuffer;
	
protected:

	virtual void UNSAFE_ON_MESSAGE_RECEAVED(BBDataBuffer* buffer){}
	virtual void UNSAFE_ON_CONNECTION_RECEAVED(){}
	
public:

	void ON_MESSAGE_RECEAVED(BBDataBuffer* buffer){UNSAFE_ON_MESSAGE_RECEAVED(buffer);}
	void ON_CONNECTION_RECEAVED(){ UNSAFE_ON_CONNECTION_RECEAVED();}

	int Write( BBDataBuffer* buffer,int offset,int count );
	void OnConnectionReceaved(DataWriter^ w);
	void Connect(int port);
	void ReceiveLoop(DataReader^ reader, StreamSocket^ socket);
	
	int GetDataBuffer(BBDataBuffer* dst, BBDataBuffer* src)
	{
		int* dst_tmp = ((int*)dst)+4;
		int* src_tmp = ((int*)src)+4;
		*dst_tmp = *src_tmp;
		*(dst_tmp +1)= *(src_tmp+1);
		return 0;
	}
		
	IBuffer^ CreateBuffer(UINT32 cbBytes, byte* ptr )
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

void BBStreamSocketListener::Connect(int port)
{
	state = 0;
	context = ref new BBStreamSocketListenerContext();
	context->SetOwner((int)this);

	dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

	listener = ref new StreamSocketListener();
	listener->Control->QualityOfService = SocketQualityOfService::Normal;

	listener->ConnectionReceived += 
			ref new TypedEventHandler<StreamSocketListener^, StreamSocketListenerConnectionReceivedEventArgs^>
			(context, &BBStreamSocketListenerContext::OnConnection);

	// Start listen operation.
	auto t = task<void>(listener->BindServiceNameAsync("1234"))
		.then([this] (task<void> previousTask)
		{
			try
			{
				// Try getting all exceptions from the continuation chain above this point.
				previousTask.get();
			}
			catch (Platform::Exception^ exception)
			{
				int j = 0;
				j++;
			}
		}).wait();
}

void BBStreamSocketListener::OnConnectionReceaved(DataWriter^ w)
{
	writer = w;
	writer->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;
	
	dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
		ref new Windows::UI::Core::DispatchedHandler([this] ()
		{
			ON_CONNECTION_RECEAVED();
		}));
		
	state = 1;
}

void BBStreamSocketListener::ReceiveLoop(DataReader^ reader, StreamSocket^ socket)
{
	task<unsigned int>(reader->LoadAsync(1024)).then([this, reader, socket] (unsigned int size)
	{
		// get data
		BBDataBuffer* buffer= new BBDataBuffer();
		buffer->_New(size);
		auto i_buffer = reader->ReadBuffer(size);
		memcpy((uint8*) buffer->ReadPointer(), (uint8*)util::GetPointer(i_buffer), size);

		// invoke main thread event handler
		dispatcher->RunAsync( CoreDispatcherPriority::Normal, 
			ref new Windows::UI::Core::DispatchedHandler([this,buffer] ()
			{
				ON_MESSAGE_RECEAVED(buffer);
			}));
		
		int blu = 0;
		blu++;

	}).then([this, reader, socket] (task<void> previousTask)
	{
		try
		{
			// Try getting all exceptions from the continuation chain above this point.
			previousTask.get();

			// Everything went ok, so try to receive another string. The receive will continue until the stream is broken (i.e. peer closed closed the socket).
			ReceiveLoop(reader, socket);
		}
		catch (Platform::Exception^ exception)
		{
			// Explicitly close the socket.
			delete socket;
		}
		catch (task_canceled&)
		{
			// Do not print anything here - this will usually happen because user closed the client socket.

			// Explicitly close the socket.
			delete socket;
		}
	});
}

void BBStreamSocketListenerContext::OnConnection(StreamSocketListener^ listener, StreamSocketListenerConnectionReceivedEventArgs^ object)
{
	DataReader^ reader = ref new DataReader(object->Socket->InputStream);
	reader->InputStreamOptions = InputStreamOptions::Partial;
	reader->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;

	owner->OnConnectionReceaved(ref new DataWriter(object->Socket->OutputStream));
	
	// Start a receive loop.
	owner->ReceiveLoop(reader, object->Socket);
}

int BBStreamSocketListener::Write( BBDataBuffer* buffer,int offset,int count ){

	if( state!=1 ) return 0;

	// write to output stream
	writer->WriteBuffer(CreateBuffer( buffer->Length(), (byte*)buffer->ReadPointer()));

	auto t = task<unsigned int>(writer->StoreAsync());
	t.then([this] (task<unsigned int> writeTask)
	{
		try
		{
			// Try getting an exception.
			writeTask.get();
		}
		catch (Object* exception)
		{
			// Send failed with error: " + exception->Message
		}
	});
	return 0;
}