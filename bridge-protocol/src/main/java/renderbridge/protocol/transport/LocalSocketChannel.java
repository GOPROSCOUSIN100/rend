package renderbridge.protocol.transport;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageHeader;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;

/**
 * Transport channel implementation using local TCP sockets.
 * Provides a fallback transport mechanism that works across all platforms.
 */
public class LocalSocketChannel implements TransportChannel {

    // Default settings
    private static final String DEFAULT_HOST = "127.0.0.1";
    private static final int DEFAULT_PORT = 25565;
    private static final long DEFAULT_TIMEOUT_MS = 1000;
    private static final int BUFFER_SIZE = 64 * 1024; // 64KB

    // Socket configuration
    private final String host;
    private final int port;
    private final String name;
    
    // Socket state
    private Socket socket;
    private ServerSocket serverSocket;
    private DataInputStream inputStream;
    private DataOutputStream outputStream;
    
    // Statistics
    private final TransportStatistics statistics = new TransportStatistics();
    
    // State
    private volatile boolean connected = false;
    private long receiveTimeoutMs = DEFAULT_TIMEOUT_MS;
    private boolean isServer;

    /**
     * Create a new local socket channel as a client.
     */
    public LocalSocketChannel() {
        this(DEFAULT_HOST, DEFAULT_PORT, false);
    }

    /**
     * Create a new local socket channel.
     *
     * @param host the host to connect to
     * @param port the port to connect to
     * @param isServer true if this is a server, false if client
     */
    public LocalSocketChannel(String host, int port, boolean isServer) {
        this.host = host;
        this.port = port;
        this.isServer = isServer;
        this.name = (isServer ? "server" : "client") + "_" + host + "_" + port;
    }

    @Override
    public void connect() throws TransportException, IOException {
        if (connected) {
            return;
        }

        try {
            if (isServer) {
                // Server mode: bind and listen
                serverSocket = new ServerSocket();
                serverSocket.bind(new InetSocketAddress(host, port));
                serverSocket.setSoTimeout((int) receiveTimeoutMs);
                
                // Accept connection
                socket = serverSocket.accept();
            } else {
                // Client mode: connect to server
                socket = new Socket();
                socket.connect(new InetSocketAddress(host, port), (int) receiveTimeoutMs);
            }
            
            // Configure socket
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
            socket.setSoTimeout((int) receiveTimeoutMs);
            
            // Create streams
            inputStream = new DataInputStream(socket.getInputStream());
            outputStream = new DataOutputStream(socket.getOutputStream());
            
            connected = true;
        } catch (IOException e) {
            disconnect();
            throw new TransportException("Failed to connect local socket channel: " + e.getMessage(), e);
        }
    }

    @Override
    public void disconnect() {
        connected = false;
        
        try {
            if (inputStream != null) {
                inputStream.close();
                inputStream = null;
            }
        } catch (IOException e) {
            // Ignore
        }
        
        try {
            if (outputStream != null) {
                outputStream.close();
                outputStream = null;
            }
        } catch (IOException e) {
            // Ignore
        }
        
        try {
            if (socket != null) {
                socket.close();
                socket = null;
            }
        } catch (IOException e) {
            // Ignore
        }
        
        try {
            if (serverSocket != null) {
                serverSocket.close();
                serverSocket = null;
            }
        } catch (IOException e) {
            // Ignore
        }
    }

    @Override
    public boolean isConnected() {
        return connected && socket != null && !socket.isClosed() && socket.isConnected();
    }

    @Override
    public void send(Message message) throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        
        try {
            ByteBuffer serialized = message.serialize();
            
            // Write message size first
            outputStream.writeInt(serialized.remaining());
            
            // Write message data
            outputStream.write(serialized.array(), serialized.arrayOffset() + serialized.position(), serialized.remaining());
            
            outputStream.flush();
            
            statistics.recordSend(serialized.remaining(), System.nanoTime() - startTime);
        } catch (IOException e) {
            statistics.recordError();
            disconnect();
            throw new TransportException("Failed to send message", e);
        }
    }

    @Override
    public void send(ByteBuffer data) throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        
        try {
            // Write data size first
            outputStream.writeInt(data.remaining());
            
            // Write data
            outputStream.write(data.array(), data.arrayOffset() + data.position(), data.remaining());
            
            outputStream.flush();
            
            statistics.recordSend(data.remaining(), System.nanoTime() - startTime);
        } catch (IOException e) {
            statistics.recordError();
            disconnect();
            throw new TransportException("Failed to send data", e);
        }
    }

    @Override
    public Message receive() throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        
        try {
            // Read message size
            int messageSize = inputStream.readInt();
            
            if (messageSize <= 0 || messageSize > BUFFER_SIZE) {
                statistics.recordError();
                throw new TransportException("Invalid message size: " + messageSize);
            }
            
            // Read message data
            byte[] messageData = new byte[messageSize];
            int bytesRead = 0;
            while (bytesRead < messageSize) {
                int read = inputStream.read(messageData, bytesRead, messageSize - bytesRead);
                if (read == -1) {
                    statistics.recordError();
                    throw new TransportException("Connection closed while reading message");
                }
                bytesRead += read;
            }
            
            // Deserialize message
            ByteBuffer messageBuffer = ByteBuffer.wrap(messageData);
            Message message = Message.deserialize(messageBuffer);
            
            statistics.recordReceive(bytesRead, System.nanoTime() - startTime);
            return message;
        } catch (IOException e) {
            statistics.recordError();
            disconnect();
            throw new TransportException("Failed to receive message", e);
        }
    }

    @Override
    public ByteBuffer receiveRaw() throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        
        try {
            // Read message size
            int messageSize = inputStream.readInt();
            
            if (messageSize <= 0 || messageSize > BUFFER_SIZE) {
                statistics.recordError();
                throw new TransportException("Invalid message size: " + messageSize);
            }
            
            // Read message data
            byte[] messageData = new byte[messageSize];
            int bytesRead = 0;
            while (bytesRead < messageSize) {
                int read = inputStream.read(messageData, bytesRead, messageSize - bytesRead);
                if (read == -1) {
                    statistics.recordError();
                    throw new TransportException("Connection closed while reading message");
                }
                bytesRead += read;
            }
            
            ByteBuffer messageBuffer = ByteBuffer.wrap(messageData);
            statistics.recordReceive(bytesRead, System.nanoTime() - startTime);
            return messageBuffer;
        } catch (IOException e) {
            statistics.recordError();
            disconnect();
            throw new TransportException("Failed to receive data", e);
        }
    }

    @Override
    public TransportType getType() {
        return TransportType.LOCAL_SOCKET;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public void setReceiveTimeout(long timeoutMillis) {
        this.receiveTimeoutMs = timeoutMillis;
        
        if (socket != null) {
            try {
                socket.setSoTimeout((int) timeoutMillis);
            } catch (SocketException e) {
                // Ignore
            }
        }
        
        if (serverSocket != null) {
            try {
                serverSocket.setSoTimeout((int) timeoutMillis);
            } catch (SocketException e) {
                // Ignore
            }
        }
    }

    @Override
    public long getReceiveTimeout() {
        return receiveTimeoutMs;
    }

    @Override
    public boolean isDataAvailable() throws IOException {
        if (!isConnected()) {
            return false;
        }
        
        if (isServer && serverSocket != null) {
            return serverSocket.getSoTimeout() > 0 && inputStream.available() > 0;
        }
        
        return inputStream.available() > 0;
    }

    @Override
    public void flush() throws IOException {
        if (outputStream != null) {
            outputStream.flush();
        }
    }

    @Override
    public TransportStatistics getStatistics() {
        return statistics;
    }

    @Override
    public void resetStatistics() {
        statistics.reset();
    }

    /**
     * Get the host this channel is connected to.
     *
     * @return the host
     */
    public String getHost() {
        return host;
    }

    /**
     * Get the port this channel is connected to.
     *
     * @return the port
     */
    public int getPort() {
        return port;
    }

    /**
     * Check if this channel is in server mode.
     *
     * @return true if this is a server
     */
    public boolean isServer() {
        return isServer;
    }

    @Override
    public String toString() {
        return String.format(
                "LocalSocketChannel{name='%s', host='%s', port=%d, connected=%s, isServer=%s}",
                name, host, port, connected, isServer
        );
    }
}
