
// BBDataBuffer to DataBuffer --> src is DataBuffer.. ??
int GetDataBuffer(BBDataBuffer* dst, BBDataBuffer* src)
{
	int* dst_tmp = ((int*)dst)+4;
	int* src_tmp = ((int*)src)+4;
	*dst_tmp = *src_tmp;
	*(dst_tmp +1)= *(src_tmp+1);
	return 0;
}
	
class NativeBuffer : public Microsoft::WRL::RuntimeClass<
						Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >,
						ABI::Windows::Storage::Streams::IBuffer,
						Windows::Storage::Streams::IBufferByteAccess >
{

public:
	virtual ~NativeBuffer()
	{
	}

	STDMETHODIMP RuntimeClassInitialize(UINT totalSize, byte* ptr)
	{
		m_length = totalSize;
		m_buffer = ptr;
		return S_OK;
	}

	STDMETHODIMP Buffer( byte **value)
	{
		*value = &m_buffer[0];
		return S_OK;
	}

		STDMETHODIMP get_Capacity(UINT32 *value)
		{
			*value = 0;
			return S_OK;
		}
                        
	STDMETHODIMP get_Length(UINT32 *value)
	{
		*value = m_length;
		return S_OK;
	}
                        
	STDMETHODIMP put_Length(UINT32 value)
	{
		m_length = value;
		return S_OK;
	}
private:
	UINT32 m_length;
	byte * m_buffer;
};



class util
{
public:
	static inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	static byte* GetPointer(IBuffer^ buffer)
	{
	   // Cast to Object^, then to its underlying IInspectable interface.

		Platform::Object^ obj = buffer;
		Microsoft::WRL::ComPtr<IInspectable> insp(reinterpret_cast<IInspectable*>(obj));

		// Query the IBufferByteAccess interface.
		Microsoft::WRL::ComPtr<IBufferByteAccess> bufferByteAccess;
		ThrowIfFailed(insp.As(&bufferByteAccess));

		// Retrieve the buffer data.
		byte* pixels = nullptr;
		ThrowIfFailed(bufferByteAccess->Buffer(&pixels));

		return pixels;
	}
	
};
