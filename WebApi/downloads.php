<?php
$directory = 'downloads';
if ($handle = opendir($directory)) {
    

    while (false !== ($entry = readdir($handle))) {
		if($entry === "index.php" || $entry === "." || $entry === "..") continue;
		$path = "$directory/$entry";
		$date = date ("F d Y H:i:s.", filemtime($path));
        echo "<a class='file-name' href='$path'>$entry</a><br/>\n<span class='download-modify-date'>(frissitve: $date)</span><br/>\n";
    }

    closedir($handle);
}
?>
