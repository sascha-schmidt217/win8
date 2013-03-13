win8
====

Windows Runtime networking features for monkey

**Interfaces**

*  StreamSocket
*  StreamSocketListener

**Notes**

*  tested with monkey v67b


**Installation**

*  copy "win8/targets/win8/" to "targets/win8/"


**Sample**

```monkey
Class StreamSocketSample Implements IStreamSocketContext, IStreamSocketListenerContext

  Field client:StreamSocket
	Field server:StreamSocketListener
	 
	Method New()
	
		server = New StreamSocketListener(IStreamSocketListenerContext(Self))
		server.Connect(1234)
		
		client = New StreamSocket(IStreamSocketContext(Self))
		client.Connect("localhost", 1234)
		
	End
	
	Method OnMessageReceaved(s:StreamSocketListener, data:DataBuffer)
	
		Print "Server: OnMessageReceaved: " + New DataStream(data).ReadLine()
		
	End 
	
	Method OnConnected(s:StreamSocketListener)
	
		Print "Server: OnConnected"
		
		Local buffer:= New DataBuffer(20)
		Local stream:= New DataStream(buffer)
		stream.WriteLine("Hallo Client..")
		s.Write(buffer,0,20)
		
	End 
	
	Method OnMessageReceaved(s:StreamSocket, data:DataBuffer)
	
		Print "Client: OnMessageReceaved: " + New DataStream(data).ReadLine()
	
	End 
	
	Method OnOpened(s:StreamSocket)
	
		Print "Client: OnOpened"
		
		Local buffer:= New DataBuffer(20)
		Local stream:= New DataStream(buffer)
		stream.WriteLine("Hallo Server..")
		s.Write(buffer,0,20)
	End 
	
	Method OnClosed(s:StreamSocket)
	End 
	
	Method OnError(s:StreamSocket)
	End 
End
```
