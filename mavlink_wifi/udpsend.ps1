$udpClient = New-Object System.Net.Sockets.UdpClient
$string = "test"
$byte = [System.Text.Encoding]::UTF8.GetBytes($string)
$udpClient.Send($byte, $byte.Length, "192.168.1.0", 14550)
$udpClient.Close()
