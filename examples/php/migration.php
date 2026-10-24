<?php
$customer = "Harry";
$orders = 3;
$label = "Customer: " . $customer;

echo $label;

if ($orders > 0) {
    echo "Orders: " . $orders;
} else {
    echo "No orders";
}

$index = 1;
while ($index <= $orders) {
    echo "Processing order " . $index;
    $index = $index + 1;
}
?>
