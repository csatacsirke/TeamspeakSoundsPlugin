<?php
header('Content-Type: text/html; charset=UTF-8');
?>

<!DOCTYPE html>
<html>

<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<style type="text/css"> 
#floating_container_id {
	position: absolute;
    right: 20px;
    top: 20px;
	border: 1px;
	border-style: solid;
	padding: 5px;
	width: 30%;
}

#container_id {
	width: 60%;
}
</style> 
</head>
<body>

<div id="container_id">

<h1>TS plugin</h1>
<!--
<p>Majd ide kerül fel mindig a legfrissebb verzió<p>
<ul>
<li>Használati utasítás: Másold be a plugin.dll-t a teamspeak /plugins mappájába </li>
<li>indítsd el a ts-t</li>
<li>felső sorban megjelenik egy plugins ablak -> katt</li>
<li>Válassz ki egy .wav (!) filet és tádá. </li>
-->
<h2>Hasznalati utasitas</h2>
<?php echo str_replace("\n", "</br>", file_get_contents("hasznalati_utasitas.txt")); ?>


<img src="tsSettings.png">
<p>(Nem vállalok felelősséget ha valami nem müködik)</p> 


</div>


<div id="floating_container_id" >


<div id="downloads_id" >
<h2>Letöltések</h2>
<?php
$directory = 'downloads';
if ($handle = opendir($directory)) {
    

    while (false !== ($entry = readdir($handle))) {
		if($entry === "index.php" || $entry === "." || $entry === "..") continue;
		$path = "$directory/$entry";
		$date = date ("F d Y H:i:s.", filemtime($path));
        echo "<a href='$path'>$entry\n</a> (frissitve: $date)<br/>";
    }

    closedir($handle);
}
?>
</div>

<div>
<h2>Változások</h2>
<?php echo str_replace("\n", "</br>", file_get_contents("changelog.txt")); ?>
</div>

</div>


</body>

</html>