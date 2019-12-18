#include "driver.h"
#include "device.tmh"


NTSTATUS
USBUMDF2DriverCreateDevice(
	_Inout_ PWDFDEVICE_INIT DeviceInit
) {
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	WDF_OBJECT_ATTRIBUTES   deviceAttributes;
	PDEVICE_CONTEXT deviceContext;
	WDFDEVICE device;
	NTSTATUS status;

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDevicePrepareHardware = USBUMDF2DriverEvtDevicePrepareHardware;
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

	if (NT_SUCCESS(status)) {
		deviceContext = DeviceGetContext(device);

		deviceContext->PrivateDeviceData = 0;

		status = WdfDeviceCreateDeviceInterface(
			device,
			&GUID_DEVINTERFACE_USBUMDF2Driver,
			NULL);

		if (NT_SUCCESS(status)) {
			status = USBUMDF2DriverQueueInitialize(device);
		}
	}

	return status;
}

NTSTATUS
USBUMDF2DriverEvtDevicePrepareHardware(
	_In_ WDFDEVICE Device,
	_In_ WDFCMRESLIST ResourceList,
	_In_ WDFCMRESLIST ResourceListTranslated
) {
	NTSTATUS status;
	PDEVICE_CONTEXT pDeviceContext;
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;

	UNREFERENCED_PARAMETER(ResourceList);
	UNREFERENCED_PARAMETER(ResourceListTranslated);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	status = STATUS_SUCCESS;
	pDeviceContext = DeviceGetContext(Device);

	if (pDeviceContext->UsbDevice == NULL) {

#if UMDF_VERSION_MINOR >= 25
		WDF_USB_DEVICE_CREATE_CONFIG createParams;

		WDF_USB_DEVICE_CREATE_CONFIG_INIT(&createParams,
			USBD_CLIENT_CONTRACT_VERSION_602);

		status = WdfUsbTargetDeviceCreateWithParameters(Device,
			&createParams,
			WDF_NO_OBJECT_ATTRIBUTES,
			&pDeviceContext->UsbDevice
		);
#else
		status = WdfUsbTargetDeviceCreate(Device,
			WDF_NO_OBJECT_ATTRIBUTES,
			&pDeviceContext->UsbDevice
		);
#endif

		if (!NT_SUCCESS(status)) {
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
				"WdfUsbTargetDeviceCreateWithParameters failed 0x%x", status);
			return status;
		}
	}

	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_MULTIPLE_INTERFACES(&configParams,
		0,
		NULL
	);
	status = WdfUsbTargetDeviceSelectConfig(pDeviceContext->UsbDevice,
		WDF_NO_OBJECT_ATTRIBUTES,
		&configParams
	);

	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfUsbTargetDeviceSelectConfig failed 0x%x", status);
		return status;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}
