#include <cstdint>

#include "asmfunc.h"
#include "pci.hpp"
#include "error.hpp"

namespace
{
	using namespace pci;

	uint32_t MakeAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_addr)
	{
		auto shl = [](uint32_t x, unsigned int bits)
		{
			return x << bits;
		};

		return shl(1, 31) | shl(bus, 16) | shl(device, 11) | shl(function, 8) | (reg_addr & 0xfcu);
	}

	Error AddDevice(const Device &device)
	{
		if (num_device == devices.size())
		{
			return MAKE_ERROR(Error::kFull);
		}

		devices[num_device] = device;
		++num_device;
		return MAKE_ERROR(Error::kSuccess);
	};

	Error ScanBus(uint8_t bus);

	Error ScanFunction(uint8_t bus, uint8_t device, uint8_t function)
	{
		auto header_type = ReadHeaderType(bus, device, function);
		auto class_code = ReadClassCode(bus, device, function);
		Device dev{bus, device, function, header_type, class_code};

		if (auto err = AddDevice(dev))
		{
			return err;
		}

		if (class_code.Match(0x06u, 0x04u))
		{
			auto bus_numbers = ReadBusNumbers(bus, device, function);
			uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
			return ScanBus(secondary_bus);
		}

		return MAKE_ERROR(Error::kSuccess);
	}

	Error ScanDevice(uint8_t bus, uint8_t device)
	{
		if (auto err = ScanFunction(bus, device, 0))
		{
			return err;
		}
		if (IsSingleFunctionDevice(ReadHeaderType(bus, device, 0)))
		{
			return MAKE_ERROR(Error::kSuccess);
		}

		for (uint8_t function = 1; function < 8; ++function)
		{
			if (ReadVendorId(bus, device, function) == 0xffffu)
			{
				continue;
			}
			if (auto err = ScanFunction(bus, device, function))
			{
				return err;
			}
		}

		return MAKE_ERROR(Error::kSuccess);
	}

	Error ScanBus(uint8_t bus)
	{
		for (uint8_t device = 0; device < 32; ++device)
		{
			if (ReadVendorId(bus, device, 0) == 0xffffu)
			{
				continue;
			}
			if (auto err = ScanDevice(bus, device))
			{
				return err;
			}
		}

		return MAKE_ERROR(Error::kSuccess);
	}
}

namespace pci
{

	void WriteAddress(uint32_t address)
	{
		IoOut32(kConfigAddress, address);
	}

	void WriteData(uint32_t value)
	{
		IoOut32(kConfigData, value);
	}

	uint32_t ReadData()
	{
		return IoIn32(kConfigData);
	}

	uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function)
	{
		WriteAddress(MakeAddress(bus, device, function, 0x00));
		return ReadData() & 0xffffu;
	}

	uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function)
	{
		WriteAddress(MakeAddress(bus, device, function, 0x00));
		return ReadData() >> 16;
	}

	uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function)
	{
		WriteAddress(MakeAddress(bus, device, function, 0x0c));
		return (ReadData() >> 16) & 0xffu;
	}

	// 2行目（07-04）を全部クラスコードって呼んでるの？
	// Revision ID と Interface も含まれていない？
	ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function)
	{
		WriteAddress(MakeAddress(bus, device, function, 0x08));
		uint32_t class_code = ReadData();

		uint8_t interface = (class_code >> 8) & 0xffu;
		uint8_t base = (class_code >> 24) & 0xffu;
		uint8_t sub = (class_code >> 16) & 0xffu;

		return ClassCode{base, sub, interface};
	}

	// 「Base Address Register 2」って書いてあるけど？？
	uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function)
	{
		WriteAddress(MakeAddress(bus, device, function, 0x18));
		return ReadData();
	}

	// なんでこれで行けるのかわかってない
	bool IsSingleFunctionDevice(uint8_t header_type)
	{
		return (header_type & 0x80u) == 0;
	}

	uint32_t ReadConfReg(const Device &device, uint8_t reg_addr)
	{
		WriteAddress(MakeAddress(device.bus, device.device, device.function, reg_addr));
		return ReadData();
	}

	void WriteConfReg(const Device &device, uint8_t reg_addr, uint32_t value)
	{
		WriteAddress(MakeAddress(device.bus, device.device, device.function, reg_addr));
		WriteData(value);
	}

	WithError<uint64_t> ReadBar(const Device &device, unsigned int bar_index)
	{
		if (bar_index >= 6)
		{
			return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
		};

		int bar_addr = 0x10 + 4 * bar_index;
		uint32_t bar = ReadConfReg(device, bar_addr);

		if ((bar & 04u) == 0)
		{
			return {bar, MAKE_ERROR(Error::kSuccess)};
		}

		uint32_t bar_upper = ReadConfReg(device, bar_addr + 4);
		uint64_t value = bar | (static_cast<uint64_t>(bar_upper) << 32);

		return WithError<uint64_t>{value, MAKE_ERROR(Error::kSuccess)};
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	Error ScanAllBus()
	{
		num_device = 0;

		auto header_type = ReadHeaderType(0, 0, 0);
		if (IsSingleFunctionDevice(header_type))
		{
			return ScanBus(0);
		}

		// バスの番号がファンクションに対応しているので
		// 0~8を回せば全部のバスをスキャンできたことになるらしい p.147
		for (uint8_t function = 1; function < 8; ++function)
		{
			if (ReadVendorId(0, 0, function) == 0xffffu)
			{
				continue;
			}
			if (auto err = ScanBus(function))
			{
				return err;
			}
		}
		return MAKE_ERROR(Error::kSuccess);
	}
}