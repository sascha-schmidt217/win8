' (c) 2013 sascha schmidt

#If TARGET<>"win8"
	#Error "win8 streamsocket"
#End 

Import util
Import brl.databuffer

Import "native/streamsocketlistener.cpp"

Extern

Class BBStreamSocketListener
	Method UNSAFE_ON_MESSAGE_RECEAVED:Void(data:BBDataBuffer)
	Method UNSAFE_ON_CONNECTION_RECEAVED:Void()
	Method Connect(port)
	Method Write(buffer:DataBuffer,offset,count )
	Method Close()
	Method GetDataBuffer(dst:DataBuffer, src:BBDataBuffer)
End 

Public 

Interface IStreamSocketListenerContext
	Method OnMessageReceaved(s:StreamSocketListener, data:DataBuffer)
	Method OnConnected(s:StreamSocketListener)
End 

Class StreamSocketListener Extends BBStreamSocketListener

	Method New(context:IStreamSocketListenerContext)
		_context = context
		_buffer = New DataBuffer
	End 
	
	#rem
	Method Socket:StreamSocket() 
		Return _socket
	End 
	#end 
	
Private

	Field _context:IStreamSocketListenerContext
	Field _buffer:DataBuffer
	
	Method UNSAFE_ON_MESSAGE_RECEAVED:Void(data:BBDataBuffer)
		If _context
		
			'' this is ugly
			
			GetDataBuffer(_buffer,data)
			
			_context.OnMessageReceaved(Self,_buffer)
		End 
	End 
	
	Method UNSAFE_ON_CONNECTION_RECEAVED:Void()
		If _context
			_context.OnConnected(Self)
		End 
	End 
	
End 
