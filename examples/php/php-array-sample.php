<?php
$items = ["A", "B", "C"];

$customer = [];
$customer["name"] = "Peter";
$customer["city"] = "Munich";
$orders = ["first" => ["total" => 10], "second" => ["total" => 20]];

echo $customer["name"];
echo $items[1];
echo count($items);
echo $orders["second"]["total"];

foreach ($items as $item) {
    echo $item;
}
foreach ($orders as $name => $order) {
    echo $name;
    echo $order["total"];
}
?>
