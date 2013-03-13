
Import mojo
Import brl.datastream
Import win8.networking.sockets

Global myapp:StreamSocketSample 
Function Main()
	myapp = New MonkeyPuzzle()
End

Class StreamSocketSample Extends App Implements IStreamSocketContext, IStreamSocketListenerContext

	Field client:StreamSocket
	Field server:StreamSocketListener
	 
	Method OnCreate()
		SetUpdateRate 60
		
		server = New StreamSocketListener(IStreamSocketListenerContext(Self))
		server.Connect(1234)
		
		client = New StreamSocket(IStreamSocketContext(Self))
		client.Connect("localhost", 1234)
		
	End
	
	Method OnMessageReceaved(s:StreamSocketListener, data:DataBuffer)
	
		Local stream:= New DataStream(data)
		Local str:= stream.ReadLine()
		
		Print "Server: OnMessageReceaved: " + str
	End 
	
	Method OnConnected(s:StreamSocketListener)
		Print "Server: OnConnected"
		
		Local buffer:= New DataBuffer(20)
		Local stream:= New DataStream(buffer)
		stream.WriteLine("Hallo Client..")
		s.Write(buffer,0,20)
		
	End 
	
	Method OnMessageReceaved(s:StreamSocket, data:DataBuffer)
	
		Local stream:= New DataStream(data)
		Local str:= stream.ReadLine()
	
		Print "Client: OnMessageReceaved: " + str
	End 
	
	Method OnOpened(s:StreamSocket)
		Print "Client: OnOpened"
		
		Local buffer:= New DataBuffer(20)
		Local stream:= New DataStream(buffer)
		stream.WriteLine("Hallo Server..")
		s.Write(buffer,0,20)
	End 
	
	Method OnClosed(s:StreamSocket)
		Print "Client: OnClosed"
	End 
	
	Method OnError(s:StreamSocket)
		Print "Client: OnError"
	End 
	
	Method OnUpdate()
	End

	Method OnRender()
		Cls 0,0,0
	End
	
End