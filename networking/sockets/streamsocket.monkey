' (c) 2013 sascha schmidt

#If TARGET<>"win8"
	#Error "win8 streamsocket"
#End 

Import util
Import brl.databuffer

Import "native/streamsocket.cpp"

Extern

Class BBStreamSocket
	Method UNSAFE_ON_MESSAGE_REVEAVED:Void(data:BBDataBuffer)
	Method UNSAFE_ON_OPENED:Void()
	Method UNSAFE_ON_CLOSED:Void()
	Method UNSAFE_ON_ERROR:Void()
	Method Connect(addr:String, port)
	Method Write(buffer:DataBuffer,offset,count )
	Method Close()
End 

Public 

Interface IStreamSocketContext
	Method OnMessageReceaved(s:StreamSocket, data:DataBuffer)
	Method OnOpened(s:StreamSocket)
	Method OnClosed(s:StreamSocket)
	Method OnError(s:StreamSocket)
End 

Class StreamSocket Extends BBStreamSocket

	Method New(context:IStreamSocketContext)
		_context = context
		_buffer = New DataBuffer
	End 
	
Private

	Field _context:IStreamSocketContext
	Field _buffer:DataBuffer
	
	Method UNSAFE_ON_MESSAGE_REVEAVED:Void(data:BBDataBuffer)
		If _context 
		
			GetDataBuffer(_buffer, data)
			
			_context.OnMessageReceaved(Self,_buffer)
		End 
	End 
	
	Method UNSAFE_ON_OPENED:Void()
		If _context 
			_context.OnOpened(Self)
		End 
	End 
	
	Method UNSAFE_ON_CLOSED:Void()
		If _context 
			_context.OnClosed(Self)
		End 
	End 
	
	Method UNSAFE_ON_ERROR:Void()
		If _context 
			_context.OnError(Self)
		End 
	End 
	
End 



