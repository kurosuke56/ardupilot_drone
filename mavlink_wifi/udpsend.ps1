$udpClient = New-Object System.Net.Sockets.UdpClient
$string = "test"
$byte = [System.Text.Encoding]::UTF8.GetBytes($string)
$udpClient.Send($byte, $byte.Length, "mydrone.local", 14550)
$udpClient.Close()
