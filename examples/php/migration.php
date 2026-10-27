<?php
$customer = "Harry";
$orders = 3;
$label = "Customer: " . $customer;

echo "customer label expected=Customer: Harry result=" . $label;

if ($orders > 0) {
    echo "order count expected=Orders: 3 result=" . "Orders: " . $orders;
} else {
    echo "order count expected=Orders: 3 result=No orders";
}

$index = 1;
while ($index <= $orders) {
    echo "order message expected=Processing order " . $index . " result=Processing order " . $index;
    $index = $index + 1;
}
?>
